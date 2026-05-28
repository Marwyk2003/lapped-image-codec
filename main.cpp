#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>

#include "quantization.hpp"
#include "transform1d.hpp"
#include "transform2d.hpp"

using namespace std;

void report(vector<double> &source, vector<double> &encoded,
            vector<double> &decoded) {
  cout << fixed << setprecision(2);
  cout << setw(15) << "Original" << setw(15) << "Encoded" << setw(15)
       << "Decoded" << endl;
  cout << "---------------------------------------------------------" << endl;

  for (size_t i = 0; i < source.size(); ++i) {
    cout << setw(13) << source[i] << setw(16) << encoded[i] << setw(15)
         << decoded[i] << endl;
  }
  cout << endl;
}

void testDct2(vector<double> &source) {
  int n = source.size();

  vector<double> encoded(source);
  dct1d::dct2(n, encoded.data());

  vector<double> decoded(encoded);
  dct1d::idct2(n, decoded.data());

  report(source, encoded, decoded);
}

void testDct4(vector<double> &source) {
  int n = source.size();

  vector<double> encoded(source);
  dct1d::dct4(n, encoded.data());

  vector<double> decoded(encoded);
  dct1d::idct4(n, decoded.data());

  report(source, encoded, decoded);
}

void test1d() {
  vector<double> source = {10, 20, 30, 40, 50, 60, 70, 80};

  testDct2(source);
  testDct4(source);
}

template <typename T> void print_matrix(int N, const vector<T> &matrix) {
  cout << fixed << setprecision(1);
  for (int r = 0; r < N; ++r) {
    for (int c = 0; c < N; ++c) {
      cout << setw(7) << +matrix[r * N + c] << " ";
    }
    cout << "\n";
  }
}

void test2d() {
  const int N = 8;

  vector<double> source(N * N);
  for (int i = 0; i < N * N; ++i) {
    source[i] = rand() % 100;
  }

  cout << "=================== ORIGINAL 8x8 IMAGE BLOCK ===================\n";
  print_matrix(N, source);
  cout << "\n";

  cout << "==================== 2D DCT COEFFICIENTS =====================\n";
  vector<double> encoded(source);
  dct2d::dct2(N, encoded.data());
  print_matrix(N, encoded);
  cout << "\n";

  cout << "=================== RECOVERED IMAGE (IDCT) ===================\n";
  vector<double> decoded(encoded);
  dct2d::idct2(N, decoded.data());
  print_matrix(N, decoded);
  cout << "\n";

  double max_error = 0.0;
  for (int i = 0; i < N * N; ++i) {
    double error = abs(source[i] - decoded[i]);
    if (error > max_error) {
      max_error = error;
    }
  }

  cout << "===================== VERIFICATION REPORT =====================\n";
  cout << "Maximum pixel reconstruction error: " << max_error << "\n";
  if (max_error < 1e-9) {
    cout << "SUCCESS: 2D DCT/IDCT round-trip completed perfectly!\n";
  } else {
    cout << "FAILURE: Discrepancy found in data recovery.\n";
  }
  cout << endl;
}

void testLT1d() {
  int N = 8 * 2;
  vector<double> source(N);
  for (int i = 0; i < N; i++) {
    source[i] = rand() % 100;
  }
  int n = source.size();

  vector<double> encoded(source);
  encode1d(n, encoded.data());

  vector<double> decoded(encoded);
  decode1d(n, decoded.data());

  report(source, encoded, decoded);
}

void testQuantization2d() {
  const int N = 8;

  vector<double> source(N * N);
  for (int i = 0; i < N * N; ++i) {
    source[i] = rand() % 100;
  }

  cout << "=================== ORIGINAL 8x8 IMAGE BLOCK ===================\n";
  print_matrix(N, source);
  cout << "\n";

  vector<double> encoded(source);
  dct2d::dct2(N, encoded.data());

  cout << "=================== QUANTIZED MATRIX ===================\n";
  auto q = quantizateBlock(N, encoded.data(), N);
  print_matrix(N, q);
  cout << "\n";

  cout << "=================== UNQUANTIZED MATRIX ===================\n";
  auto dq = dequantizateBlock(N, q.data(), N);
  print_matrix(N, dq);
  cout << "\n";

  cout << "=================== RECOVERED IMAGE ===================\n";
  vector<double> decoded(dq);
  dct2d::idct2(N, decoded.data());
  print_matrix(N, decoded);
  cout << "\n";

  double max_error = 0.0;
  for (int i = 0; i < N * N; ++i) {
    double error = abs(source[i] - decoded[i]);
    if (error > max_error) {
      max_error = error;
    }
  }

  cout << "===================== VERIFICATION REPORT =====================\n";
  cout << "Maximum pixel reconstruction error: " << max_error << "\n";
  if (max_error < 1) {
    cout << "SUCCESS: 2D quantization round-trip completed perfectly!\n";
  } else {
    cout << "FAILURE: Discrepancy found in data recovery.\n";
  }
  cout << endl;
}

void testLT2d() {
  int N = 8 * 2;
  vector<double> source(N * N);
  for (int i = 0; i < N * N; i++) {
    source[i] = rand() % 100;
  }
  int n = source.size();

  vector<double> encoded(source);
  encode2d(N, encoded.data(), N);

  vector<double> decoded(encoded);
  decode2d(N, decoded.data(), N);

  report(source, encoded, decoded);
}

int main() {
  // test1d();
  // test2d();
  // testLT1d();
  // testLT2d();
  // testQuantization2d();

  return 0;
}
