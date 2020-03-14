#!/bin/sh
cd Dependencies/bullet3

if [ -e CMakeCache.txt ]; then
  rm CMakeCache.txt
fi
mkdir -p build_cmake_debug
cd build_cmake_debug
# cmake -DBUILD_PYBULLET=ON -DBUILD_PYBULLET_NUMPY=ON -DUSE_DOUBLE_PRECISION=ON -DBT_USE_EGL=ON -DCMAKE_BUILD_TYPE=Debug .. || exit 1
# Macoy was here: Use single precision instead
# Pybullet is necessary for world serialization stuff (why?)
cmake -DBUILD_PYBULLET=ON -DBUILD_PYBULLET_NUMPY=ON -DBT_USE_EGL=ON -DCMAKE_BUILD_TYPE=Debug \
	  -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libstdc++" \
	  -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libstdc++" \
	  .. || exit 1
# cmake -DBT_USE_EGL=ON -DCMAKE_BUILD_TYPE=Debug .. || exit 1
make -j $(command nproc 2>/dev/null || echo 12) || exit 1

echo "Completed build of Bullet."
