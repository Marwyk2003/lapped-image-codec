#include "metrics.hpp"
#include "quantization.hpp"
#include "quantizedImageCodec.hpp"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Summary {
  string example;
  bool lapped;
  int strength;
  string quantizer;

  double mseAvg;
  double ssimulacra;
  double compressionRatio;
};

template <class QZ, bool Lapped, int Strength>
Summary runCodecDemo(const string &example, const string &ext = ".jpg") {
  cerr << "Running example: " << example << " for " << "lapped=" << Lapped
       << ", strength=" << Strength << ", quantizer=" << QZ::name << '\n';

  string outputDir = "output/" + example + "/";
  std::filesystem::create_directory(outputDir);

  stringstream inputPathStream, dctPathStream, decodedPathStream;
  string type = Lapped ? "lapped" : "block";
  string inputPath = "images/" + example + ext;
  string dctPath = outputDir + example + "_" + type + "_" + QZ::name + "_" +
                   to_string(Strength) + ".dct";
  string decodedPath = outputDir + example + "_" + type + "_" + QZ::name + "_" +
                       to_string(Strength) + "_decoded.png";

  Image original = loadImage(inputPath.c_str());
  int rawBytes = original.width * original.height * original.channels.size();

  QuantizedImageCodec<QZ, Lapped, Strength>::encode(inputPath.c_str(),
                                                    dctPath.c_str());
  int encodedBytes = std::filesystem::file_size(dctPath.c_str());

  Image decoded =
      QuantizedImageCodec<QZ, Lapped, Strength>::decode(dctPath.c_str());

  double mseSum = 0.0;
  for (int i = 0; i < original.channels.size(); ++i) {
    auto m = mse(original.channels[i], decoded.channels[i]);
    mseSum += m;
  }
  double mseAvg = mseSum / original.channels.size();

  double ssimulacra = 0.0;
  if (original.channels.size() == 1) {
    ssimulacra =
        zensim_ssimulacra2_grayscale(original.channels[0], decoded.channels[0],
                                     original.width, original.height);
  } else if (original.channels.size() == 3) {
    ssimulacra = zensim_ssimulacra2_rgb(original.channels, decoded.channels,
                                        original.width, original.height);
  }

  double compressionRatio = 100.0 * encodedBytes / rawBytes;

  saveImage(decodedPath.c_str(), decoded);

  return Summary{
      .example = example,
      .lapped = Lapped,
      .strength = Strength,
      .quantizer = QZ::name,
      .mseAvg = mseAvg,
      .ssimulacra = ssimulacra,
      .compressionRatio = compressionRatio,
  };
}

int main() {
  vector<string> examples = {"barbara", "panda_small"};
  vector<int> strengths = {1, 10, 100};

  vector<Summary> summary;

  /// BLOCK
  for (auto &e : examples) {
    /// BLOCK
    summary.push_back(runCodecDemo<Quantizer8Base, false, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, false, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, false, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, false, 1>(e));

    summary.push_back(runCodecDemo<Quantizer8Base, false, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, false, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, false, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, false, 10>(e));

    summary.push_back(runCodecDemo<Quantizer8Base, false, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, false, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, false, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, false, 100>(e));

    /// LAPPED
    summary.push_back(runCodecDemo<Quantizer8Base, true, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, true, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, true, 1>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, true, 1>(e));

    summary.push_back(runCodecDemo<Quantizer8Base, true, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, true, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, true, 10>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, true, 10>(e));

    summary.push_back(runCodecDemo<Quantizer8Base, true, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Pow2, true, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Tiled, true, 100>(e));
    summary.push_back(runCodecDemo<Quantizer16Upscaled, true, 100>(e));
  }

  cout << "\n" << setfill('=') << setw(100) << "" << endl;
  cout << setfill(' ');
  cout << left << setw(15) << "Image" << setw(10) << "Type" << setw(10)
       << "Strength" << setw(25) << "Quantizer" << setw(12) << "MSE Avg"
       << setw(12) << "SSIM2" << setw(12) << "Ratio" << endl;
  cout << setw(100) << setfill('-') << "" << setfill(' ') << endl;

  for (const auto &s : summary) {
    cout << left << setw(15) << s.example << setw(10)
         << (s.lapped ? "lapped" : "block") << setw(10) << s.strength
         << setw(25) << s.quantizer << setw(12) << fixed << setprecision(3)
         << s.mseAvg << setw(12) << setprecision(5) << s.ssimulacra << setw(8)
         << setprecision(2) << s.compressionRatio << "%" << endl;
  }
  cout << setw(100) << setfill('=') << "" << endl;

  return 0;
}
