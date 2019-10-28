#!/bin/sh
echo Building Bullet3...
cd bullet3 && ./build_cmake_pybullet_double.sh
echo Finished building Bullet3!

echo
echo Building Base2.0...
# TODO: Fix this foolishness
cd base2.0 && jam -j4 && jam -j4 libBase20
echo Finished building Base2.0!

echo Building Horde3D...
cd ../Horde3D && mkdir build && cd build && cmake .. && make -j4
echo Finished building Horde3D!
