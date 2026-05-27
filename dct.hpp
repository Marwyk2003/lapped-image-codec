#include <complex>
#include <vector>

constexpr double PI = 3.14159265358979323846;

inline void assertPow2(int n) {
  if ((n & (n - 1)) != 0) {
    throw std::invalid_argument("Array size must be a power of 2.");
  }
}

void fft(int n, std::complex<double> *x, bool inv) {
  assertPow2(n);

  if (n == 1)
    return;

  std::vector<std::complex<double>> even(n / 2), odd(n / 2);
  for (int i = 0; i < n / 2; ++i) {
    even[i] = x[i * 2];
    odd[i] = x[i * 2 + 1];
  }

  fft(n / 2, even.data(), inv);
  fft(n / 2, odd.data(), inv);

  double ang = inv ? 2 * PI / n : -2 * PI / n;
  for (int i = 0; i < n / 2; ++i) {
    auto t = std::polar(1.0, ang * i) * odd[i];
    x[i] = even[i] + t;
    x[i + n / 2] = even[i] - t;
    if (inv) {
      x[i] /= 2;
      x[i + n / 2] /= 2;
    }
  }
}

void fft(int n, std::complex<double> *x) { fft(n, x, false); }

void ifft(int n, std::complex<double> *x) { fft(n, x, true); }

namespace dct1d {
void dct2(int n, double *x) {
  assertPow2(n);

  std::vector<std::complex<double>> y(n);
  for (int i = 0; i < n / 2; ++i) {
    y[i] = x[2 * i];
    y[n - 1 - i] = x[2 * i + 1];
  }

  fft(n, y.data());

  for (int i = 0; i < n; ++i) {
    auto t = std::polar(1.0, -PI * i / (2.0 * n));
    x[i] = 2.0 * (y[i] * t).real();
  }
}

void idct2(int n, double *x) {
  assertPow2(n);

  std::vector<std::complex<double>> y(n);
  y[0] = 0.5 * x[0];
  for (int i = 1; i < n; ++i) {
    std::complex<double> c(x[i], -x[n - i]);
    std::complex<double> t = std::polar(1.0, PI * i / (2.0 * n));
    y[i] = 0.5 * c * t;
  }

  ifft(n, y.data());

  for (int i = 0; i < n / 2; ++i) {
    x[2 * i] = y[i].real();
    x[2 * i + 1] = y[n - 1 - i].real();
  }
}

void dct4(int n, double *x) {
  assertPow2(n);

  std::vector<std::complex<double>> y(n / 2);
  for (int i = 0; i < n / 2; ++i) {
    std::complex<double> val(x[2 * i], x[n - 1 - 2 * i]);
    std::complex<double> t = std::polar(1.0, -PI * (i + 0.25) / n);
    y[i] = val * t;
  }

  fft(n / 2, y.data());

  for (int i = 0; i < n / 2; ++i) {
    std::complex<double> t = std::polar(1.0, -PI * (i + 0.25) / n);
    std::complex<double> yi = y[i] * t;

    x[2 * i] = yi.real();
    x[n - 1 - 2 * i] = -yi.imag();
  }
}

void idct4(int n, double *x) {
  dct4(n, x);
  for (int i = 0; i < n; ++i) {
    x[i] /= 2;
  }
}
} // namespace dct1d

namespace dct2d {
void dct2(int n, double **x) {
  assertPow2(n);

  std::vector<double> row(n);
  for (int r = 0; r < n; ++r) {
    for (int c = 0; c < n; ++c) {
      row[c] = x[r][c];
    }
    dct1d::dct2(n, row.data());
    for (int c = 0; c < n; ++c) {
      x[r][c] = row[c];
    }
  }

  std::vector<double> col(n);
  for (int c = 0; c < n; ++c) {
    for (int r = 0; r < n; ++r) {
      col[r] = x[r][c];
    }
    dct1d::dct2(n, col.data());
    for (int r = 0; r < n; ++r) {
      x[r][c] = col[r];
    }
  }
}

void idct2(int n, double **x) {
  assertPow2(n);

  std::vector<double> col(n);
  for (int c = 0; c < n; ++c) {
    for (int r = 0; r < n; ++r) {
      col[r] = x[r][c];
    }
    dct1d::idct2(n, col.data());
    for (int r = 0; r < n; ++r) {
      x[r][c] = col[r];
    }
  }

  std::vector<double> row(n);
  for (int r = 0; r < n; ++r) {
    for (int c = 0; c < n; ++c) {
      row[c] = x[r][c];
    }
    dct1d::idct2(n, row.data());
    for (int c = 0; c < n; ++c) {
      x[r][c] = row[c];
    }
  }
}
} // namespace dct2d
