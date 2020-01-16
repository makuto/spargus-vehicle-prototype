#!/bin/sh
jam clean
cd Dependencies/bullet3 && rm -r build_cmake
rm -r build_cmake_debug
cd ../../Dependencies/base2.0 && jam clean
rm -r ../Horde3D/build
# TODO: Add Horde clean
