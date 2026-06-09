#include "quantization.hpp"
#include "quantizeImage.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

constexpr double sharpness = 2.0;

template <class QZ>
std::vector<unsigned char>
computeNormalizedError(const std::vector<unsigned char> &original,
                       const std::vector<unsigned char> &restored, int width,
                       int height) {
  constexpr int N = QZ::blockSize;
  std::vector<unsigned char> normalizedError(width * height, 0);

  int blocksX = (width + N - 1) / N;
  int blocksY = (height + N - 1) / N;

  for (int by = 0; by < blocksY; ++by) {
    for (int bx = 0; bx < blocksX; ++bx) {
      std::vector<double> errors(N * N, 0.0);
      double maxErr = -std::numeric_limits<double>::infinity();

      for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx >= width || yy >= height)
            continue;

          double err = std::abs(static_cast<double>(original[yy * width + xx]) -
                                static_cast<double>(restored[yy * width + xx]));
          errors[y * N + x] = err;
          maxErr = std::max(maxErr, err);
        }
      }

      for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
          int yy = by * N + y;
          int xx = bx * N + x;
          if (xx >= width || yy >= height)
            continue;

          double value = 1.0;
          if (maxErr > 0.0) {
            double ratio = errors[y * N + x] / maxErr;
            value =
                1.0 -
                std::pow(
                    ratio,
                    sharpness); // sharpness is constexpr at the top of the file
          }
          normalizedError[yy * width + xx] =
              static_cast<unsigned char>(std::round(value * 255.0));
          ;
        }
      }
    }
  }

  return normalizedError;
}

template <class QZ>
void getNormalizedErrorDist(const char *inputPath,
                            const char *reconstructedPath,
                            const char *errorPath, bool lapped,
                            int qscale = 1) {
  std::cout << "Processing " << inputPath << "\n";

  Image img = loadImage(inputPath);

  Image errorImg;
  errorImg.width = img.width;
  errorImg.height = img.height;
  errorImg.channels.resize(img.channels.size());

  std::cout << "size: " << img.width << "x" << img.height << "\n";

  for (int i = 0; i < img.channels.size(); ++i) {
    auto &channel = img.channels[i];
    std::vector<unsigned char> original(channel.begin(), channel.end());
    channel = quantizeChannel<QZ>(img, i, lapped, qscale);
    errorImg.channels[i] =
        computeNormalizedError<QZ>(original, channel, img.width, img.height);
  }

  saveImage(reconstructedPath, img);
  saveImage(errorPath, errorImg);
  std::cout << "reconstructed -> " << reconstructedPath << "\n";
  std::cout << "normalized error -> " << errorPath << "\n";
}

int main() {
  const char *input = "images/barbara.jpg";

  getNormalizedErrorDist<Quantizer8Base>(
      input, "output/error_dist_reconstructed_block.png",
      "output/error_dist_error_block.png", false, 10);
  std::cout << "---------------------------------" << "\n";
  getNormalizedErrorDist<Quantizer8Base>(
      input, "output/error_dist_reconstructed_lapped.png",
      "output/error_dist_error_lapped.png", true, 7);

  return 0;
}
