#pragma once

#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

double mse(const std::vector<unsigned char> &a,
           const std::vector<unsigned char> &b) {
  if (a.size() != b.size())
    throw std::invalid_argument("mse: buffer sizes differ");

  if (a.empty())
    return 0.0;

  const double sum = std::transform_reduce(
      a.begin(), a.end(), b.begin(), 0.0, std::plus<>{},
      [](unsigned char x, unsigned char y) {
        const double d = static_cast<double>(x) - static_cast<double>(y);
        return d * d;
      });
  return sum / a.size();
}
