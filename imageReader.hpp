#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>
#include <string>
#include <utility>

struct Image {
  unsigned char *data = nullptr;
  int width = 0;
  int height = 0;
  int channels = 0;

  ~Image() {
    if (data)
      stbi_image_free(data);
  }

  Image() = default;
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  Image(Image &&other) noexcept { *this = std::move(other); }

  Image &operator=(Image &&other) noexcept {
    if (this != &other) {
      if (data)
        stbi_image_free(data);
      data = other.data;
      width = other.width;
      height = other.height;
      channels = other.channels;
      other.data = nullptr;
      other.width = other.height = other.channels = 0;
    }
    return *this;
  }
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

void saveGrayscaleImage(const char *path, int width, int height,
                        const unsigned char *pixels, int stride = 0) {
  if (stride == 0)
    stride = width;
  stbi_write_png(path, width, height, 1, pixels, stride);
}
