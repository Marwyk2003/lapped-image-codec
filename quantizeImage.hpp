#pragma once

#include "dct.hpp"
#include "imageReader.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

template <class QZ>
void quantizeImage(const char *inputPath, const char *outputPath) {
  constexpr int N = QZ::blockSize;

  Image img = loadImage(inputPath);
  std::cout << "Processing " << inputPath << " -> " << outputPath << "\n";
  std::cout << "  size: " << img.width << "x" << img.height << "\n";

  int blocksX = img.width / N;
  int blocksY = img.height / N;

  std::vector<unsigned char> restored(img.width * img.height, 0);

  for (int by = 0; by < blocksY; ++by) {
    std::cerr << "  row " << by << "/" << blocksY << "\n";
    for (int bx = 0; bx < blocksX; ++bx) {
      std::vector<double> source(N * N);
      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          source[y * N + x] = img.data[yy * img.width + xx];
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
          restored[yy * img.width + xx] = static_cast<unsigned char>(
              std::clamp(decoded[y * N + x], 0.0, 255.0));
        }
      }
    }
  }

  stbi_write_png(outputPath, img.width, img.height, 1, restored.data(),
                 img.width);
  stbi_image_free(img.data);
}
