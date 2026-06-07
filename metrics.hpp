#pragma once

#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

extern "C" int zensim_score_gray(const unsigned char *a, const unsigned char *b,
                                 unsigned int width, unsigned int height,
                                 double *out_score);

extern "C" int
zensim_score_rgb(const unsigned char *a_r, const unsigned char *a_g,
                 const unsigned char *a_b, const unsigned char *b_r,
                 const unsigned char *b_g, const unsigned char *b_b,
                 unsigned int width, unsigned int height, double *out_score);

double mse(const std::vector<unsigned char> &a,
           const std::vector<unsigned char> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("mse: buffer sizes differ");

  if (a.empty())
    return 0.0;

  const double sum =
      std::transform_reduce(a.begin(), a.end(), b.begin(), 0.0, std::plus<>{},
                            [](unsigned char x, unsigned char y) {
                              const double d = x - y;
                              return d * d;
                            });
  return sum / a.size();
}

double zensim_ssimulacra2_grayscale(const std::vector<unsigned char> &a,
                                    const std::vector<unsigned char> &b,
                                    int width, int height) {
  if (a.size() != b.size())
    throw std::invalid_argument(
        "zensim_ssimulacra2_grayscale: buffer sizes differ");

  double score = 0.0;
  const int rc = zensim_score_gray(a.data(), b.data(), width, height, &score);
  if (rc != 0)
    throw std::runtime_error("zensim_ssimulacra2_grayscale: metric failed");

  return score;
}

double zensim_ssimulacra2_rgb(const std::vector<std::vector<unsigned char>> &a,
                              const std::vector<std::vector<unsigned char>> &b,
                              int width, int height) {
  if (a.size() != 3 || b.size() != 3)
    throw std::invalid_argument("zensim_ssimulacra2_rgb: expected 3 channels");

  double score = 0.0;
  const int rc =
      zensim_score_rgb(a[0].data(), a[1].data(), a[2].data(), b[0].data(),
                       b[1].data(), b[2].data(), width, height, &score);
  if (rc != 0)
    throw std::runtime_error("zensim_ssimulacra2_rgb: metric failed");

  return score;
}
