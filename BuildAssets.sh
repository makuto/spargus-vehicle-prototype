#!/bin/sh

echo "This script is now deprecated!"
echo "Use jam instead"

# echo "Exporting all .blend files in assets..."
# echo "(create Horde Collada and Bullet Obj)"
# cd assets
# for blendFile in *.blend
# do
# 	echo "---------------------------------------------"
# 	echo "$blendFile"
# 	blender $blendFile --background --python ExportBlenderAssets.py
# done
# echo "Done\n"

# echo "============================================="
# echo "Copying obj output to Collision..."
# rsync -a --stats *.obj ../Collision/
# echo "Done\n"

# echo "============================================="
# echo "Copying textures to Content..."
# rsync -a --stats *.png ../Content/textures/
# rsync -a --stats *.jpg ../Content/textures/
# echo "Done\n"

# cd ..

# echo "============================================="
# echo "Copying Collada output to Horde Content format..."
# ./Horde3D/build/Binaries/Linux/Debug/ColladaConv assets -type model -addModelName -dest Content
# echo "Done"
