#include <cmath>
#include <iostream>
#include <vector>

constexpr int quantizationMatrix[8][8] = {
    {3, 5, 7, 9, 11, 13, 15, 17},     {5, 7, 9, 11, 13, 15, 17, 19},
    {7, 9, 11, 13, 15, 17, 19, 21},   {9, 11, 13, 15, 17, 19, 21, 23},
    {11, 13, 15, 17, 19, 21, 23, 25}, {13, 15, 17, 19, 21, 23, 25, 27},
    {15, 17, 19, 21, 23, 25, 27, 29}, {17, 19, 21, 23, 25, 27, 29, 31}};

constexpr int threshold = 100; // TODO

std::vector<int> quantizateBlock(int N, double *data, int stride) {
  std::vector<int> result(N * N, 0);
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      int val = round(data[y * stride + x] / quantizationMatrix[y % 8][x % 8]);
      // TODO take top left into account instead of hardocding treshold
      if (abs(val) >= threshold) {
        result[y * N + x] = val;
      }
    }
  }
  return result;
}

std::vector<double> dequantizateBlock(int N, int *data, int stride) {
  std::vector<double> result(N * N);
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      result[y * N + x] =
          data[y * stride + x] * quantizationMatrix[y % 8][x % 8];
    }
  }
  return result;
}
