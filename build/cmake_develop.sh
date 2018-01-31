#!/bin/sh
rm -rf CMakeCache.txt CMakeFiles Makefile cmake_install.cmake
cmake -DCMAKE_TOOLCHAIN_FILE="../ksdk/tools/cmake_toolchain_files/armgcc.cmake" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Develop  .
