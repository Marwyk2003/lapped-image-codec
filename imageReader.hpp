#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct Image {
  std::vector<std::vector<unsigned char>> channels;
  int width = 0;
  int height = 0;

  Image() = default;
  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  Image(Image &&other) noexcept { *this = std::move(other); }

  Image &operator=(Image &&other) noexcept {
    if (this != &other) {
      width = other.width;
      height = other.height;
      other.width = other.height;
      channels = std::move(other.channels);
      other.channels.clear();
    }
    return *this;
  }
};

Image loadImage(const char *path) {
  Image img;
  int channels;
  auto *data = stbi_load(path, &img.width, &img.height, &channels, 0);

  if (data == nullptr) {
    throw std::runtime_error("Error: Failed to load image. " +
                             std::string(stbi_failure_reason()));
  }

  img.channels.resize(channels);
  for (int c = 0; c < channels; ++c) {
    img.channels[c].resize(img.width * img.height);
    for (int i = 0; i < img.width * img.height; ++i) {
      img.channels[c][i] = data[channels * i + c];
    }
  }

  stbi_image_free(data);

  return img;
}

void saveImage(const char *path, const Image &img) {
  int channels = img.channels.size();

  std::vector<unsigned char> flattend(img.width * img.height * channels, 0);
  for (int c = 0; c < channels; ++c) {
    for (int i = 0; i < img.width * img.height; ++i) {
      flattend[i * channels + c] = img.channels[c][i];
    }
  }

  stbi_write_png(path, img.width, img.height, channels, flattend.data(),
                 img.width * channels);
}
