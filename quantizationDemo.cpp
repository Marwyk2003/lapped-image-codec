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
void quantizeImage(const char *inputPath, const char *outputPath) {
  std::cout << "Processing " << inputPath << " -> " << outputPath << "\n";

  Image img = loadImage(inputPath);
  std::vector<unsigned char> original(img.data,
                                      img.data + img.width * img.height);

  auto restored = quantizeGrayscale<QZ>(img);

  std::cout << "  size: " << img.width << "x" << img.height << "\n";
  std::cout << "  MSE: " << mse(original, restored) << "\n";
  std::cout << "  SSIMULACRA2: "
            << zensim_ssimulacra2_grayscale(original, restored, img.width,
                                            img.height)
            << "\n";

  saveGrayscaleImage(outputPath, img.width, img.height, restored.data());
}

int main() {
  const char *input = "images/barbara.jpg";

  quantizeImage<Quantizer16Tiled>(input, "output/quantized_tiled.png");
  quantizeImage<Quantizer16Pow2>(input, "output/quantized_pow2.png");
  quantizeImage<Quantizer16Upscaled>(input, "output/quantized_upscaled.png");

  return 0;
}
