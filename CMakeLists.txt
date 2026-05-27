cmake_minimum_required(VERSION 3.14)
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

add_executable(imageDemo imageDemo.cpp)

target_include_directories(imageDemo PRIVATE ${stb_SOURCE_DIR})
