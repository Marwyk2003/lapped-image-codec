#pragma once

#include "dct.hpp"
#include "imageReader.hpp"

#include <algorithm>
#include <vector>

template <class QZ>
std::vector<unsigned char> quantizeGrayscale(const Image &img) {
  constexpr int N = QZ::blockSize;

  std::vector<unsigned char> restored(img.width * img.height, 0);

  int blocksX = (img.width + N - 1) / N;
  int blocksY = (img.height + N - 1) / N;

  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      std::vector<double> source(N * N);
      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          int cy = std::min(yy, img.height - 1);
          int cx = std::min(xx, img.width - 1);
          source[y * N + x] = img.data[cy * img.width + cx];
        }
      }

      std::vector<double> encoded(source);
      dct2d::dct2(N, encoded.data());
      auto q = QZ::quantizateBlock(encoded.data(), 100);
      auto dq = QZ::dequantizateBlock(q.data());
      std::vector<double> decoded(dq);
      dct2d::idct2(N, decoded.data());

      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx < img.width && yy < img.height) {
            restored[yy * img.width + xx] = static_cast<unsigned char>(
                std::clamp(decoded[y * N + x], 0.0, 255.0));
          }
        }
      }
    }
  }

  return restored;
}
