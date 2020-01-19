#!/bin/sh
echo Building Bullet3...
cd Dependencies/bullet3 && ./build_cmake_pybullet_double.sh
echo Finished building Bullet3!

echo
echo Building Base2.0...
# TODO: Fix this foolishness
cd Dependencies/base2.0 && jam -j4 && jam -j4 libBase20
echo Finished building Base2.0!

echo Building Horde3D...
cd Dependencies/Horde3D && mkdir build && cd build && cmake .. && make -j4
echo Finished building Horde3D!

echo Building polyvox...
cd Dependencies/polyvox && mkdir build_release && cd build_release && cmake .. -DCMAKE_BUILD_TYPE="Release" && make -j4
echo Finished building polyvox! 
