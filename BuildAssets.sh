#!/bin/sh

# Horde3D ColladaConv - 1.0.0

# Usage:
# ColladaConv input [optional arguments]

# input             asset file or directory to be processed
# -type model|anim  asset type to be processed (default: model)
# -base path        base path where the repository root is located
# -dest path        existing destination path where output is written
# -noGeoOpt         disable geometry optimization
# -overwriteMats    force update of existing materials
# -addModelName     adds model name before material name
# -lodDist1 dist    distance for LOD1
# -lodDist2 dist    distance for LOD2
# -lodDist3 dist    distance for LOD3
# -lodDist4 dist    distance for LOD4

./Horde3D/build/Binaries/Linux/Debug/ColladaConv assets -type model -addModelName -dest Content
