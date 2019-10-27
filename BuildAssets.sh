#!/bin/sh

echo "Exporting all .blend files in assets..."
echo "(create Horde Collada and Bullet Obj)"
cd assets
for blendFile in *.blend
do
	echo "---------------------------------------------"
	echo "$blendFile"
	blender $blendFile --background --python ExportBlenderAssets.py
done
echo "Done\n"

cd ..

echo "============================================="
echo "Copying Collada output to Horde Content format..."
./Horde3D/build/Binaries/Linux/Debug/ColladaConv assets -type model -addModelName -dest Content
echo "Done"
