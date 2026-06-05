#pragma once

#include "imageReader.hpp"

#include <vector>

struct Partition {
  const Image &img;
  int blockSize;
  int blocksX, blocksY;
  int channels, strideBytes;
  std::vector<std::vector<unsigned char *>> data;

  Partition(const Image &image, int blockSize)
      : img(image), blockSize(blockSize) {
    blocksX = img.width / blockSize;
    blocksY = img.height / blockSize;
    channels = img.channels;
    strideBytes = img.width * img.channels;

    data.resize(blocksY, std::vector<unsigned char *>(blocksX));
    for (int by = 0; by < blocksY; ++by) {
      for (int bx = 0; bx < blocksX; ++bx) {
        int xx = bx * blockSize;
        int yy = by * blockSize;
        data[by][bx] = img.data + (yy * img.width + xx) * img.channels;
      }
    }
  }
};
