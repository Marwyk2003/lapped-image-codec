#pragma once

#include "dct.hpp"
#include "imageReader.hpp"
#include "transform2d.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

template <typename T>
void getBlock(const std::vector<T> &data, int bx, int by, int N, int width,
              T *block) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int xx = bx * N + j;
      int yy = by * N + i;
      block[i * N + j] = data[yy * width + xx];
    }
  }
}

template <typename T>
void setBlock(std::vector<T> &data, int bx, int by, int N, int width,
              const T *block) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int xx = bx * N + j;
      int yy = by * N + i;
      data[yy * width + xx] = block[i * N + j];
    }
  }
}

template <class QZ>
std::vector<unsigned char> quantizeChannel(const Image &img, int channel,
                                           bool lapped, int qscale) {
  constexpr int N = QZ::blockSize;

  auto data = img.channels[channel];
  std::vector<unsigned char> restored(img.width * img.height, 0);

  int blocksX = (img.width + N - 1) / N;
  int blocksY = (img.height + N - 1) / N;

  int width = blocksX * N;
  int height = blocksY * N;

  std::vector<double> source(width * height);
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          int cy = std::min(yy, img.height - 1);
          int cx = std::min(xx, img.width - 1);
          source[yy * width + xx] = (int)data[cy * img.width + cx] - 128;
        }
      }
    }
  }

  std::vector<double> encoded(source);
  if (lapped) {
    prefilter2d(width, height, N, encoded.data());
  }
  std::array<double, QZ::blockSize * QZ::blockSize> block{};
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      getBlock(encoded, bx, by, N, width, block.data());
      dct2d::dct2(N, block.data());
      setBlock(encoded, bx, by, N, width, block.data());
    }
  }

  std::vector<int> q(width * height);
  std::array<int, QZ::blockSize * QZ::blockSize> qBlock{};
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      getBlock(encoded, bx, by, N, width, block.data());
      QZ::quantizateBlock(block.data(), qscale, qBlock.data());
      setBlock(q, bx, by, N, width, qBlock.data());
    }
  }

  int count = 0;
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      if (q[y * width + x] != 0)
        ++count;
  std::cout << "Non-zero coefficients: "
            << ((double)count / (width * height)) * 100 << "%" << std::endl;

  std::vector<double> dq(width * height);
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      getBlock(q, bx, by, N, width, qBlock.data());
      QZ::dequantizateBlock(qBlock.data(), qscale, block.data());
      setBlock(dq, bx, by, N, width, block.data());
    }
  }

  std::vector<double> decoded(dq);
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      getBlock(decoded, bx, by, N, width, block.data());
      dct2d::idct2(N, block.data());
      setBlock(decoded, bx, by, N, width, block.data());
    }
  }
  if (lapped) {
    postfliter2d(width, height, N, decoded.data());
  }

  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx < img.width && yy < img.height) {
            restored[yy * img.width + xx] = static_cast<int>(
                std::clamp(decoded[yy * width + xx] + 128.0, 0.0, 255.0));
          }
        }
      }
    }
  }

  return restored;
}
