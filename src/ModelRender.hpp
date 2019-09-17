#pragma once

#include "tiny_gltf.h"

typedef int GLCallListIndex;

void drawModel(const tinygltf::Model& model);
GLCallListIndex buildCallListFromModel(const tinygltf::Model& model);
GLCallListIndex buildCallListFromModel2(const tinygltf::Model& model);

void initModel(const tinygltf::Model& model);

// void drawModel2(const tinygltf::Model& model);
