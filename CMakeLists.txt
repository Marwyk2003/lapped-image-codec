cmake_minimum_required(VERSION 3.22)
project(LappedImageCodec)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG        31c1ad37456438565541f4919958214b6e762fb4
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
  Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG        v0.6
)
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(
  MANIFEST_PATH ${CMAKE_SOURCE_DIR}/third_party/zensim_ffi/Cargo.toml
)

add_executable(quantizationDemo quantizationDemo.cpp)
add_executable(normalizedErrorDemo normalizedErrorDemo.cpp)

target_include_directories(quantizationDemo PRIVATE ${stb_SOURCE_DIR})
target_include_directories(normalizedErrorDemo PRIVATE ${stb_SOURCE_DIR})

target_link_libraries(quantizationDemo PRIVATE zensim_ffi)
if(UNIX)
  target_link_libraries(quantizationDemo PRIVATE m pthread dl)
endif()
