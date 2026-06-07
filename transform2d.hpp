#pragma once

#include "transform1d.hpp"

void encode2d(int w, int h, int blockSize, double *data) {
  std::vector<double> row(w);
  for (int r = 0; r < h; ++r) {
    for (int c = 0; c < w; ++c) {
      row[c] = data[r * w + c];
    }
    encode1d(w, blockSize, row.data());
    for (int c = 0; c < w; ++c) {
      data[r * w + c] = row[c];
    }
  }

  std::vector<double> col(h);
  for (int c = 0; c < w; ++c) {
    for (int r = 0; r < h; ++r) {
      col[r] = data[r * w + c];
    }
    encode1d(h, blockSize, col.data());
    for (int r = 0; r < h; ++r) {
      data[r * w + c] = col[r];
    }
  }
}

void decode2d(int w, int h, int blockSize, double *data) {
  {
    std::vector<double> col(h);
    for (int c = 0; c < w; ++c) {
      for (int r = 0; r < h; ++r) {
        col[r] = data[r * w + c];
      }
      decode1d(h, blockSize, col.data());
      for (int r = 0; r < h; ++r) {
        data[r * w + c] = col[r];
      }
    }

    std::vector<double> row(w);
    for (int r = 0; r < h; ++r) {
      for (int c = 0; c < w; ++c) {
        row[c] = data[r * w + c];
      }
      decode1d(w, blockSize, row.data());
      for (int c = 0; c < w; ++c) {
        data[r * w + c] = row[c];
      }
    }
  }
}
