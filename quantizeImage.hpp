#pragma once

#include "dct.hpp"
#include "imageReader.hpp"
#include "transform2d.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

template <typename T>
std::vector<T> getBlock(const std::vector<T> &data, int bx, int by, int N,
                        int width) {
  std::vector<T> block(N * N);
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int xx = bx * N + j;
      int yy = by * N + i;
      block[i * N + j] = data[yy * width + xx];
    }
  }
  return block;
}

template <typename T>
void setBlock(std::vector<T> &data, int bx, int by, int N, int width,
              const std::vector<T> &block) {
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int xx = bx * N + j;
      int yy = by * N + i;
      data[yy * width + xx] = block[i * N + j];
    }
  }
}

template <class QZ>
std::vector<unsigned char> quantizeGrayscale(const Image &img, bool lapped) {
  constexpr int N = QZ::blockSize;
  int qscale = lapped ? 7 : 10; // TODO: hardcoded to match compression rate

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
          source[yy * width + xx] = (int)img.data[cy * img.width + cx] - 128;
        }
      }
    }
  }

  std::vector<double> encoded(source);
  if (lapped) {
    prefilter2d(width, height, N, encoded.data());
  }
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      auto b = getBlock(encoded, bx, by, N, width);
      dct2d::dct2(N, b.data());
      setBlock(encoded, bx, by, N, width, b);
    }
  }

  std::vector<int> q(width * height);
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      auto b1 = getBlock(encoded, bx, by, N, width);
      auto b2 = QZ::quantizateBlock(b1.data(), qscale);
      setBlock(q, bx, by, N, width, b2);
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
      auto b1 = getBlock(q, bx, by, N, width);
      auto b2 = QZ::dequantizateBlock(b1.data(), qscale);
      setBlock(dq, bx, by, N, width, b2);
    }
  }

  std::vector<double> decoded(dq);
  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      auto b = getBlock(decoded, bx, by, N, width);
      dct2d::idct2(N, b.data());
      setBlock(decoded, bx, by, N, width, b);
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
