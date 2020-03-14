#!/bin/sh
echo Building Bullet3...
./BuildBullet_Debug.sh
echo Finished building Bullet3!

echo
echo Building Base2.0...
# TODO: Fix this foolishness
# TODO: Add debug build
cd Dependencies/base2.0 && ls && ./BuildDependencies_Debug.sh \
	&& jam -j4 -sDEBUG_BUILD=true && jam -j4 -sDEBUG_BUILD=true libBase20
echo Finished building Base2.0!

echo Building Horde3D...
cd ../Horde3D && mkdir build && cd build \
	&& cmake .. -DCMAKE_BUILD_TYPE="Debug" \
			 -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libstdc++" \
			 -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libstdc++" \
	&& make -j4
echo Finished building Horde3D!
