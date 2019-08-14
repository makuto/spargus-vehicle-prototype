#!/bin/sh
jam clean
cd bullet3 && rm -r build_cmake
cd ../base2.0 && jam clean
