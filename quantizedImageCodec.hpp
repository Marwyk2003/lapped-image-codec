#pragma once

#include "dct.hpp"
#include "imageReader.hpp"
#include "quantizeImage.hpp"
#include "transform2d.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>
#include <vector>

template <class QZ, bool Lapped, int Strength> struct QuantizedImageCodec {
  static constexpr int N = QZ::blockSize;

  static constexpr int maskWordCount() { return (N * N + 63) / 64; }

  static bool isMaskBitSet(const std::array<uint64_t, maskWordCount()> &mask,
                           int index) {
    return (mask[index / 64] & (uint64_t(1) << (index % 64))) != 0;
  }

  static void setMaskBit(std::array<uint64_t, maskWordCount()> &mask,
                         int index) {
    mask[index / 64] |= uint64_t(1) << (index % 64);
  }

  static void buildMask(const int *block,
                        std::array<uint64_t, maskWordCount()> &mask) {
    mask.fill(0);
    for (int i = 0; i < N * N; ++i) {
      if (block[i] != 0)
        setMaskBit(mask, i);
    }
  }

  template <class T> static void writeData(std::ostream &os, const T &value) {
    os.write(reinterpret_cast<const char *>(&value), sizeof(T));
  }

  template <class T> static void readData(std::istream &is, T &value) {
    is.read(reinterpret_cast<char *>(&value), sizeof(T));
  }

  static void writeBlock(std::ostream &os, const int *block) {
    std::array<uint64_t, maskWordCount()> mask{};
    buildMask(block, mask);

    for (uint64_t word : mask)
      writeData(os, word);

    for (int i = 0; i < N * N; ++i) {
      if (block[i] != 0)
        writeData(os, block[i]);
    }
  }

  static void readBlock(std::istream &is, int *block) {
    std::array<uint64_t, maskWordCount()> mask{};

    for (uint64_t &word : mask)
      readData(is, word);

    std::fill(block, block + N * N, 0);
    for (int i = 0; i < N * N; ++i) {
      if (isMaskBitSet(mask, i))
        readData(is, block[i]);
    }
  }

  static std::vector<double> padData(const std::vector<unsigned char> &data,
                                     int imgWidth, int imgHeight) {
    int blocksX = (imgWidth + N - 1) / N;
    int blocksY = (imgHeight + N - 1) / N;
    int width = blocksX * N;
    std::vector<double> source(blocksX * blocksY * N * N);

    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        for (int x = 0; x < N; ++x) {
          for (int y = 0; y < N; ++y) {
            int yy = by * N + y;
            int xx = bx * N + x;
            int cy = std::min(yy, imgHeight - 1);
            int cx = std::min(xx, imgWidth - 1);
            source[yy * width + xx] =
                static_cast<double>((data[cy * imgWidth + cx])) - 128;
          }
        }
      }
    }
    return source;
  }

  static void encodeChannel(std::ostream &os,
                            const std::vector<unsigned char> &data,
                            int imgWidth, int imgHeight) {
    std::vector<double> rawData = padData(data, imgWidth, imgHeight);

    int blocksX = (imgWidth + N - 1) / N;
    int blocksY = (imgHeight + N - 1) / N;
    int width = blocksX * N;
    int height = blocksY * N;

    if constexpr (Lapped)
      prefilter2d(width, height, N, rawData.data());

    std::array<double, N * N> block{};
    std::array<int, N * N> q{};
    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        getBlock(rawData, bx, by, N, width, block.data());
        dct2d::dct2(N, block.data());
        QZ::quantizateBlock(block.data(), Strength, q.data());
        writeBlock(os, q.data());
      }
    }
  }

  static std::vector<unsigned char> decodeChannel(std::istream &is,
                                                  int imgWidth, int imgHeight) {
    int blocksX = (imgWidth + N - 1) / N;
    int blocksY = (imgHeight + N - 1) / N;
    int width = blocksX * N;
    int height = blocksY * N;

    std::vector<int> coefficientsData(width * height, 0);
    std::array<int, N * N> block{};
    std::array<double, N * N> restoredBlock{};
    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        readBlock(is, block.data());
        setBlock(coefficientsData, bx, by, N, width, block.data());
      }
    }

    std::vector<double> decodedData(width * height);
    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        getBlock(coefficientsData, bx, by, N, width, block.data());
        QZ::dequantizateBlock(block.data(), Strength, restoredBlock.data());
        dct2d::idct2(N, restoredBlock.data());
        setBlock(decodedData, bx, by, N, width, restoredBlock.data());
      }
    }

    if constexpr (Lapped)
      postfliter2d(width, height, N, decodedData.data());

    std::vector<unsigned char> restored(imgWidth * imgHeight, 0);
    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        for (int x = 0; x < N; ++x) {
          for (int y = 0; y < N; ++y) {
            int yy = by * N + y;
            int xx = bx * N + x;
            if (xx < imgWidth && yy < imgHeight) {
              restored[yy * imgWidth + xx] = static_cast<unsigned char>(
                  std::clamp(decodedData[yy * width + xx] + 128.0, 0.0, 255.0));
            }
          }
        }
      }
    }
    return restored;
  }

  static void encode(const char *inputPath, const char *outputPath) {
    Image img = loadImage(inputPath);

    std::ofstream os(outputPath, std::ios::binary);
    if (!os)
      throw std::runtime_error("QuantizedImageCodec: failed to open output");

    writeData(os, static_cast<uint32_t>(img.width));
    writeData(os, static_cast<uint32_t>(img.height));
    writeData(os, static_cast<uint32_t>(img.channels.size()));

    for (int i = 0; i < img.channels.size(); ++i)
      encodeChannel(os, img.channels[i], img.width, img.height);
  }

  static Image decode(const char *inputPath) {
    std::ifstream is(inputPath, std::ios::binary);
    if (!is)
      throw std::runtime_error("QuantizedImageCodec: failed to open input");

    uint32_t width, height, channels;
    readData(is, width);
    readData(is, height);
    readData(is, channels);

    Image img(width, height, channels);

    for (int i = 0; i < channels; ++i)
      img.channels[i] = decodeChannel(is, width, height);

    return img;
  }

  static void decodeToFile(const char *inputPath, const char *outputPngPath) {
    Image img = decode(inputPath);
    saveImage(outputPngPath, img);
  }
};
