# Lapped Image Codec

## Deps

- **CMake**
- **C++20** compiler
- **Rust**

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Run quantization demo

```bash
./build/quantizationDemo
```

> [!NOTE]
> Expects writable `./output` directory.

## Metrics

- **MSE** — mean squared error on grayscale pixel vector
- **zensim_ssimulacra2_grayscale** — perceptual similarity score (0–100, higher is better) via [zensim](https://github.com/imazen/zensim)
