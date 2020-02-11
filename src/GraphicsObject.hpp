#pragma once

#include <glm/mat4x4.hpp>  // mat4

namespace Graphics
{
// To be defined by render library
class ResourceReference;

class Object
{
public:
	Object();

	const glm::mat4& GetTransformConstRef() const;
	glm::mat4 GetTransformCopy() const;
	glm::vec3 GetPosition() const;

	void SetTransform(const glm::mat4& newTransform);

	void Initialize(const char* requestedResource);

	// Remove the graphics object from the scene
	// TODO: Should this be how it works? This sucks not being in the destructor. I did this because
	// this class needs move stuff in order to properly handle ownership...
	void Destroy();

protected:
	glm::mat4 transform;

	// To be defined by render library
	void TransformUpdated();
	ResourceReference* resource;
};

// TODO: Figure out this API...
class ProceduralMesh : public Object
{
public:
	void Initialize(const char* geoName, float* vertices, unsigned int* indices,
	                // Optional
	                short* normals, short* tangents, short* bitangents, float* texture1UVs,
	                float* texture2UVs,
	                // Required
	                int numVertices, int numIndices);
};
}  // namespace Graphics
