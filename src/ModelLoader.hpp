#pragma once

namespace tinygltf
{
	class Model;
}

bool LoadModelFromGltf(const char* filename, tinygltf::Model& modelOut);
