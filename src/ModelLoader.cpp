#include "ModelLoader.hpp"

#include <string>
#include <iostream>

// Define these only in *one* file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "tiny_gltf.h"

bool LoadModelFromGltf(const char* filename, tinygltf::Model& modelOut)
{
	tinygltf::TinyGLTF loader;
	std::string error;
	std::string warning;

	// bool succeeded = loader.LoadASCIIFromFile(&modelOut, &error, &warning, filename);

	// for binary glTF(.glb)
	bool succeeded = loader.LoadBinaryFromFile(&modelOut, &error, &warning, filename);

	if (!warning.empty())
	{
		std::cout << "Warn: " << warning << "\n";
	}

	if (!error.empty())
	{
		std::cout << "Err: " << error << "\n";
	}

	if (!succeeded)
	{
		std::cout << "Failed to parse glTF\n";
		return false;
	}
	
	return true;
}
