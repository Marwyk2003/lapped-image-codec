#pragma once

#include "dct.hpp"

void butterfly(int n, double *data) {
  for (int i = 0; i < n / 2; i++) {
    double u = data[i], v = data[n - 1 - i];
    data[i] = u + v;
    data[n - 1 - i] = u - v;
  }
}

void jswap(int n, double *data) {
  for (int i = 0; i < n / 2; i++) {
    std::swap(data[i], data[n - 1 - i]);
  }
}

void div(int n, double *data, int x) {
  for (int i = 0; i < n; i++) {
    data[i] /= x;
  }
}

void encode1d(int n, double *data) {
  int m = 8;

  for (int i = 0; i < n / m - 1; i++) {
    double *block = data + m / 2 + i * m;
    double *lower = block + m / 2;

    /* pre-filtering */
    butterfly(m, block);

    /* V */
    jswap(m / 2, lower);
    dct1d::dct4(m / 2, lower);
    dct1d::idct2(m / 2, lower);
    jswap(m / 2, lower);
    /* V */

    butterfly(m, block);
    div(m, block, 2);
    /* pre-filtering */
  }

  for (int i = 0; i < n / m; i++) {
    double *block = data + i * m;
    dct1d::dct2(m, block);
  }
}

void decode1d(int n, double *data) {
  int m = 8;

  for (int i = 0; i < n / m; i++) {
    double *block = data + i * m;
    dct1d::idct2(m, block);
  }

  for (int i = 0; i < n / m - 1; i++) {
    double *block = data + m / 2 + i * m;
    double *lower = block + m / 2;

    /* post-filtering */
    butterfly(m, block);

    /* V^-1 */
    jswap(m / 2, lower);
    dct1d::dct2(m / 2, lower);
    dct1d::idct4(m / 2, lower);
    jswap(m / 2, lower);
    /* V^-1 */

    butterfly(m, block);
    div(m, block, 2);
    /* post-filtering */
  }
}
