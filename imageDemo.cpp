#include <iomanip>

#include "dct.hpp"
#include "imageReader.hpp"
#include "iostream"
#include "quantization.hpp"

using namespace std;

template <typename T> void print_matrix(int N, const vector<T> &matrix) {
  cout << fixed << setprecision(1);
  for (int r = 0; r < N; ++r) {
    for (int c = 0; c < N; ++c) {
      cout << setw(7) << +matrix[r * N + c] << " ";
    }
    cout << "\n";
  }
}

int main() {
  Image img = loadImage("images/barbara.jpg");
  cout << "Image width: " << img.width << endl;
  cout << "Image height: " << img.height << endl;
  cout << "Image channels: " << img.channels << endl;

  int N = 16;
  int blocksX = img.width / N;
  int blocksY = img.height / N;

  vector<unsigned char> restored(img.width * img.height, 0);

  for (int by = 0; by < blocksY; ++by) {
    cerr << "Processing row " << by << "/" << blocksY << endl;
    for (int bx = 0; bx < blocksX; ++bx) {
      vector<double> source(N * N);
      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          source[y * N + x] = img.data[yy * img.width + xx];
        }
      }

      vector<double> encoded(source);
      dct2d::dct2(N, encoded.data());
      auto q = quantizateBlock(N, encoded.data(), N);
      if (bx == 0 && by == 0) {
        print_matrix(N, q);
      }
      auto dq = dequantizateBlock(N, q.data(), N);
      vector<double> decoded(dq);
      dct2d::idct2(N, decoded.data());

      for (int x = 0; x < N; ++x) {
        for (int y = 0; y < N; ++y) {
          int yy = by * N + y;
          int xx = bx * N + x;
          restored[yy * img.width + xx] = decoded[y * N + x];
        }
      }
    }
  }

  stbi_write_png("output/quantized.png", img.width, img.height, 1, restored.data(),
                 img.width);

  stbi_image_free(img.data);

  return 0;
}
