#include "quantization.hpp"
#include "quantizeImage.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

constexpr double sharpness = 2.0;

template <class QZ>
void normalizedErrorDemo(const char *inputPath, const char *reconstructedPath,
                         const char *errorPath) {
  std::cout << "Processing " << inputPath << "\n";

  Image img = loadImage(inputPath);
  std::vector<unsigned char> original(img.data,
                                      img.data + img.width * img.height);

  auto restored = quantizeGrayscale<QZ>(img);

  std::cout << "  size: " << img.width << "x" << img.height << "\n";

  saveGrayscaleImage(reconstructedPath, img.width, img.height, restored.data());

  constexpr int N = QZ::blockSize;
  std::vector<double> normalizedError(img.width * img.height, 0.0);

  int blocksX = (img.width + N - 1) / N;
  int blocksY = (img.height + N - 1) / N;

  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      std::vector<double> errors(N * N, 0.0);
      double maxErr = -std::numeric_limits<double>::infinity();

      for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx >= img.width || yy >= img.height)
            continue;

          double err =
              std::abs(static_cast<double>(original[yy * img.width + xx]) -
                       static_cast<double>(restored[yy * img.width + xx]));
          errors[y * N + x] = err;
          maxErr = std::max(maxErr, err);
        }
      }

      for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx >= img.width || yy >= img.height)
            continue;

          double value = 1.0;
          if (maxErr > 0.0) {
            double ratio = errors[y * N + x] / maxErr;
            value = 1.0 - std::pow(ratio, sharpness);
          }
          normalizedError[yy * img.width + xx] = value;
        }
      }
    }
  }

  std::vector<unsigned char> errorImage(img.width * img.height);
  for (int i = 0; i < img.width * img.height; ++i) {
    errorImage[i] =
        static_cast<unsigned char>(std::round(normalizedError[i] * 255.0));
  }

  saveGrayscaleImage(errorPath, img.width, img.height, errorImage.data());
  std::cout << "  reconstructed -> " << reconstructedPath << "\n";
  std::cout << "  normalized error -> " << errorPath << "\n";
}

int main() {
  const char *input = "images/barbara.jpg";

  constexpr int strength = 128;
  normalizedErrorDemo<Quantizer<Q16Pow2, strength>>(
      input, "output/error_dist_reconstructed.png",
      "output/error_dist_error.png");

  return 0;
}
