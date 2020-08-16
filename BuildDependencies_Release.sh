#!/bin/sh
echo Building Bullet3...
./BuildBullet.sh || exit 1
echo Finished building Bullet3!

echo Building Base2.0...
# TODO: Fix this foolishness
cd Dependencies/base2.0
./BuildDependencies_Release.sh || exit 1
jam -j4 && jam -j4 libBase20 || exit 1
cd ../../
echo Finished building Base2.0!

echo Building Horde3D...
cd Dependencies/Horde3D
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE="Release" \
         -DCMAKE_CXX_COMPILER=clang++ \
         -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libstdc++" -DCMAKE_SHARED_LINKER_FLAGS="-stdlib=libstdc++" || exit 1
# I am not sure what the implications of this are
# -DOpenGL_GL_PREFERENCE="LEGACY" \
# Note that -j4 etc. will break Horde make, because it doesn't like out of order execution!
make || exit 1
cd ../../
echo Finished building Horde3D!

# Refer to https://raw.githubusercontent.com/OGRECave/ogre-next/master/Scripts/BuildScripts/output/build_ogre_linux_c%2B%2Blatest.sh
echo "Building Ogre..."
cd Ogre/ogre-next
cd build/Release
cmake -D OGRE_USE_BOOST=0 -D OGRE_CONFIG_THREAD_PROVIDER=0 -D OGRE_CONFIG_THREADS=0 -D OGRE_BUILD_COMPONENT_SCENE_FORMAT=1 -D OGRE_BUILD_SAMPLES2=1 -D OGRE_BUILD_TESTS=1 -D CMAKE_BUILD_TYPE="Release"  -G Ninja ../.. || exit $?
ninja || exit $?
echo "Finished building Ogre"
