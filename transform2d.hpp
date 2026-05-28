#pragma once

#include "transform1d.hpp"

void encode2d(int n, double *data, int stride) {
  std::vector<double> row(n);
  for (int r = 0; r < n; ++r) {
    for (int c = 0; c < n; ++c) {
      row[c] = data[r * stride + c];
    }
    encode1d(n, row.data());
    for (int c = 0; c < n; ++c) {
      data[r * stride + c] = row[c];
    }
  }

  std::vector<double> col(n);
  for (int c = 0; c < n; ++c) {
    for (int r = 0; r < n; ++r) {
      col[r] = data[r * stride + c];
    }
    encode1d(n, col.data());
    for (int r = 0; r < n; ++r) {
      data[r * stride + c] = col[r];
    }
  }
}

void decode2d(int n, double *data, int stride) {
  std::vector<double> col(n);
  for (int c = 0; c < n; ++c) {
    for (int r = 0; r < n; ++r) {
      col[r] = data[r * stride + c];
    }
    decode1d(n, col.data());
    for (int r = 0; r < n; ++r) {
      data[r * stride + c] = col[r];
    }
  }

  std::vector<double> row(n);
  for (int r = 0; r < n; ++r) {
    for (int c = 0; c < n; ++c) {
      row[c] = data[r * stride + c];
    }
    decode1d(n, row.data());
    for (int c = 0; c < n; ++c) {
      data[r * stride + c] = row[c];
    }
  }
}
