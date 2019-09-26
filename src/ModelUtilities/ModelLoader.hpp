#pragma once

#include <string>
#include <vector>

#include "material.h"
#include "mesh.h"

namespace gltf
{
bool LoadGLTF(const std::string& filename, float scale, std::vector<Mesh<float>>* meshes,
              std::vector<Material>* materials, std::vector<Texture>* textures, bool debugOutput);
}
