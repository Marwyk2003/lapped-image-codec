#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>
#include <string>
#include <vector>

struct Image {
  unsigned char *data;
  int width;
  int height;
  int channels;
};

Image loadImage(const char *path) {
  Image img;
  img.data = stbi_load(path, &img.width, &img.height, &img.channels, 1);
  img.channels = 1;

  if (img.data == nullptr) {
    throw std::runtime_error("Error: Failed to load image. " +
                             std::string(stbi_failure_reason()));
  }

  return img;
}

struct Partition {
  int blockSize;

  int blocksX, blocksY;
  int channels, strideBytes;

  Image img;
  std::vector<std::vector<unsigned char *>> data;

  Partition(Image &image, int blockSize) : blockSize(blockSize), img(image) {
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

void savePartition(Partition &p, char *filename) {
  for (int by = 0; by < p.blocksY; ++by) {
    for (int bx = 0; bx < p.blocksX; ++bx) {
      std::string filename = "output/blocks_" + std::to_string(by) + "_" +
                             std::to_string(bx) + ".png";
      stbi_write_png(filename.c_str(), p.blockSize, p.blockSize, p.channels,
                     p.data[by][bx], p.strideBytes);
    }
  }
}
