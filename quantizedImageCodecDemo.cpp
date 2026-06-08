#include "metrics.hpp"
#include "quantization.hpp"
#include "quantizedImageCodec.hpp"

#include <filesystem>
#include <iostream>

template <class QZ, bool Lapped, int Strength>
void runCodecDemo(const char *inputPath, const char *dctPath,
                  const char *decodedPath) {
  std::cout << "Codec demo"
            << " (lapped=" << (Lapped ? "true" : "false")
            << ", strength=" << Strength << ")\n";
  std::cout << "  encode: " << inputPath << " -> " << dctPath << "\n";

  Image original = loadImage(inputPath);
  int rawBytes = original.width * original.height * original.channels.size();

  QuantizedImageCodec<QZ, Lapped, Strength>::encode(inputPath, dctPath);
  int encodedBytes = std::filesystem::file_size(dctPath);

  std::cout << "  decode: " << dctPath << " -> " << decodedPath << "\n";
  Image decoded = QuantizedImageCodec<QZ, Lapped, Strength>::decode(dctPath);

  for (int i = 0; i < original.channels.size(); ++i) {
    std::cout << "  channel " << i << ":\n";
    std::cout << "    MSE: " << mse(original.channels[i], decoded.channels[i])
              << "\n";
  }

  if (original.channels.size() == 1) {
    std::cout << "  grayscale SSIMULACRA2: "
              << zensim_ssimulacra2_grayscale(original.channels[0],
                                              decoded.channels[0],
                                              original.width, original.height)
              << "\n";
  } else if (original.channels.size() == 3) {
    std::cout << "  RGB SSIMULACRA2: "
              << zensim_ssimulacra2_rgb(original.channels, decoded.channels,
                                        original.width, original.height)
              << "\n";
  }

  std::cout << "  raw bytes: " << rawBytes << "\n";
  std::cout << "  encoded bytes: " << encodedBytes << "\n";
  std::cout << "  compression ratio: " << 100.0 * encodedBytes / rawBytes
            << "%\n";
}

int main() {
  const char *input = "images/panda.jpg";

  runCodecDemo<Quantizer8Base, false, 10>(input, "output/panda_block.dct",
                                          "output/panda_block_decoded.png");
  std::cout << "---------------------------------\n";
  runCodecDemo<Quantizer8Base, true, 7>(input, "output/panda_lapped.dct",
                                        "output/panda_lapped_decoded.png");

  return 0;
}
