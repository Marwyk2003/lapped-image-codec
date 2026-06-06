#pragma once

#include <array>
#include <cmath>
#include <vector>

template <int N, class T = int> using Mat = std::array<std::array<T, N>, N>;

consteval Mat<8, int> getBase8x8() {
  return Mat<8, int>{{{3, 5, 7, 9, 11, 13, 15, 17},
                      {5, 7, 9, 11, 13, 15, 17, 19},
                      {7, 9, 11, 13, 15, 17, 19, 21},
                      {9, 11, 13, 15, 17, 19, 21, 23},
                      {11, 13, 15, 17, 19, 21, 23, 25},
                      {13, 15, 17, 19, 21, 23, 25, 27},
                      {15, 17, 19, 21, 23, 25, 27, 29},
                      {17, 19, 21, 23, 25, 27, 29, 31}}};
}

template <int N, int Factor, class T = int>
consteval Mat<N * Factor, T> tile(const Mat<N, T> &m) {
  constexpr int dstN = N * Factor;
  Mat<dstN, T> result{};
  for (int y = 0; y < dstN; ++y) {
    for (int x = 0; x < dstN; ++x) {
      result[y][x] = m[y % N][x % N];
    }
  }
  return result;
}

template <int N> consteval Mat<N, int> zigzagRank() {
  Mat<N, int> rank{};
  int idx = 0;
  for (int sum = 0; sum < 2 * N - 1; ++sum) {
    if ((sum & 1) == 0) {
      int y0 = (sum < N) ? sum : (N - 1);
      int y1 = (sum < N) ? 0 : (sum - N + 1);
      for (int y = y0; y >= y1; --y) {
        rank[y][sum - y] = idx++;
      }
    } else {
      int y0 = (sum < N) ? 0 : (sum - N + 1);
      int y1 = (sum < N) ? sum : (N - 1);
      for (int y = y0; y <= y1; ++y) {
        rank[y][sum - y] = idx++;
      }
    }
  }
  return rank;
}

template <int N> consteval int powerOf2FromRank(int r) {
  if (r < 4)
    return 1;
  if (r < 9)
    return 2;
  constexpr int tail = N * N - 9;
  constexpr int bandSize = tail > 0 ? (tail + 5) / 6 : 1;
  int band = (r - 9) / bandSize;
  int shift = 2 + band;
  if (shift > 7)
    shift = 7;
  return 1 << shift;
}

template <int N> consteval Mat<N, int> getPowerOf2() {
  constexpr auto zz = zigzagRank<N>();
  Mat<N, int> result{};
  for (int y = 0; y < N; ++y) {
    for (int x = 0; x < N; ++x) {
      result[y][x] = powerOf2FromRank<N>(zz[y][x]);
    }
  }
  return result;
}

constexpr double interpolate(double a, double b, double p) {
  return a * (1 - p) + b * p;
}

template <int N, int Factor, class T = int>
consteval Mat<N * Factor, T> upsample(const Mat<N, T> &q) {
  constexpr int dstN = N * Factor;
  Mat<dstN, T> result{};
  for (int y = 0; y < dstN; ++y) {
    double py = y * static_cast<double>(N - 1) / (dstN - 1);
    int y0 = static_cast<int>(py);
    py -= y0;
    int y1 = (y0 < N - 1) ? (y0 + 1) : (N - 1);

    for (int x = 0; x < dstN; ++x) {
      double px = x * static_cast<double>(N - 1) / (dstN - 1);
      int x0 = static_cast<int>(px);
      px -= x0;
      int x1 = (x0 < N - 1) ? (x0 + 1) : (N - 1);

      double a = q[y0][x0];
      double b = q[y0][x1];
      double c = q[y1][x0];
      double d = q[y1][x1];

      double ab = interpolate(a, b, px);
      double cd = interpolate(c, d, px);
      double v = interpolate(ab, cd, py);

      result[y][x] = static_cast<T>(v + 0.5);
    }
  }
  return result;
}

struct Q8Base {
  static constexpr int N = 8;
  static constexpr Mat<N, int> Q = getBase8x8();
};

struct Q16TiledFromQ8Base {
  static constexpr int N = 16;
  static constexpr Mat<N, int> Q = tile<8, 2>(getBase8x8());
};

struct Q16UpscaledFromQ8Base {
  static constexpr int N = 16;
  static constexpr Mat<N, int> Q = upsample<8, 2>(getBase8x8());
};

struct Q16Pow2 {
  static constexpr int N = 16;
  static constexpr Mat<N, int> Q = getPowerOf2<N>();
};

template <class QProvider, int Strength> struct Quantizer {
  static_assert(Strength >= 1, "Strength must be >= 1");

  static constexpr int blockSize = QProvider::N;
  static constexpr Mat<blockSize, int> Q = QProvider::Q;
  static constexpr int strength = Strength;

  static std::vector<int> quantizateBlock(const double *data, int qscale = 1) {
    std::vector<int> result(blockSize * blockSize, 0);
    for (int y = 0; y < blockSize; ++y) {
      for (int x = 0; x < blockSize; ++x) {
        int val = std::round(data[y * blockSize + x] / (Q[y][x] * Strength));
        result[y * blockSize + x] = val;
      }
    }
    return result;
  }

  static std::vector<double> dequantizateBlock(const int *data,
                                               int qscale = 1) {
    std::vector<double> result(blockSize * blockSize);
    for (int y = 0; y < blockSize; ++y) {
      for (int x = 0; x < blockSize; ++x) {
        result[y * blockSize + x] =
            static_cast<double>(data[y * blockSize + x]) * (Q[y][x] * Strength);
      }
    }
    return result;
  }
};

using Quantizer8Base = Quantizer<Q8Base, 1>;
using Quantizer16Tiled = Quantizer<Q16TiledFromQ8Base, 1>;
using Quantizer16Upscaled = Quantizer<Q16UpscaledFromQ8Base, 1>;
using Quantizer16Pow2 = Quantizer<Q16Pow2, 1>;
