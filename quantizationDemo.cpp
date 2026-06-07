#include "metrics.hpp"
#include "quantization.hpp"
#include "quantizeImage.hpp"

#include <iostream>

template <int N, class T>
consteval bool matricesEqual(const Mat<N, T> &a, const Mat<N, T> &b) {
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      if (a[y][x] != b[y][x]) {
        return false;
      }
    }
  }
  return true;
}

static_assert(matricesEqual<4>(tile<2, 2>(Mat<2, int>{{{10, 20}, {30, 40}}}),
                               Mat<4, int>{{
                                   {{10, 20, 10, 20}},
                                   {{30, 40, 30, 40}},
                                   {{10, 20, 10, 20}},
                                   {{30, 40, 30, 40}},
                               }}));

static_assert(matricesEqual<4>(zigzagRank<4>(), Mat<4, int>{{
                                                    {{0, 1, 5, 6}},
                                                    {{2, 4, 7, 12}},
                                                    {{3, 8, 11, 13}},
                                                    {{9, 10, 14, 15}},
                                                }}));

static_assert(matricesEqual<4>(upsample<2, 2>(Mat<2, int>{{{1, 3}, {2, 4}}}),
                               Mat<4, int>{{
                                   {{1, 2, 2, 3}},
                                   {{1, 2, 3, 3}},
                                   {{2, 2, 3, 4}},
                                   {{2, 3, 3, 4}},
                               }}));

static_assert(matricesEqual<4>(getPowerOf2<4>(), Mat<4, int>{{
                                                     {{1, 1, 2, 2}},
                                                     {{1, 2, 2, 8}},
                                                     {{1, 2, 8, 16}},
                                                     {{4, 4, 16, 32}},
                                                 }}));

template <class QZ>
void quantizeImage(const char *inputPath, const char *outputPath, bool lapped) {
  std::cout << "Processing " << inputPath << " -> " << outputPath << "\n";

  Image img = loadImage(inputPath);
  for (int i = 0; i < img.channels.size(); ++i) {
    auto &channel = img.channels[i];
    std::vector<unsigned char> original(channel.begin(), channel.end());
    channel = quantizeChannel<QZ>(img, i, lapped);

    std::cout << "  size: " << img.width << "x" << img.height << "\n";
    std::cout << "  MSE: " << mse(original, channel) << "\n";
    std::cout << "  SSIMULACRA2: "
              << zensim_ssimulacra2_grayscale(original, channel, img.width,
                                              img.height)
              << "\n";
  }

  saveGrayscaleImage(outputPath, img);
}

int main() {
  const char *input = "images/panda.jpg";

  quantizeImage<Quantizer8Base>(input, "output/quantized_block.png", false);
  std::cout << "---------------------------------" << "\n";
  quantizeImage<Quantizer8Base>(input, "output/quantized_lapped.png", true);

  return 0;
}
