# Designed for C++ to JavaScript transpiling
# -DCMAKE_TOOLCHAIN_FILE="$CHEERP_PATH/share/cmake/Modules/CheerpToolchain.cmake"
cmake_minimum_required(VERSION 3.14)

project(libcrails-router
  LANGUAGES CXX
  VERSION 2.0.0)

include_directories(./libcrails-router)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/libcrails-router/crails"
        DESTINATION "include"
        FILES_MATCHING
        PATTERN "*.hpp")

