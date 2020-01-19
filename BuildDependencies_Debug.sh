#!/bin/sh
echo Building Bullet3...
./build_cmake_pybullet_double_debug.sh
echo Finished building Bullet3!

echo
echo Building Base2.0...
# TODO: Fix this foolishness
# TODO: Add debug build
cd Dependencies/base2.0 && jam -j4 && jam -j4 libBase20
echo Finished building Base2.0!

echo Building Horde3D...
cd ../Horde3D && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE="Debug" && make -j4
echo Finished building Horde3D!

echo Building polyvox...
cd Dependencies/polyvox && mkdir build_debug && cd build_debug && cmake .. -DCMAKE_BUILD_TYPE="Debug" && make -j4
echo Finished building polyvox! 
