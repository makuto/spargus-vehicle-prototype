#pragma once

#include <glm/mat4x4.hpp>         // mat4

namespace Graphics
{
// To be defined by render library
class NodeResourceReference;

class Node
{
public:
	const glm::mat4& GetTransformConstRef() const;
	glm::mat4 GetTransformCopy() const;

	void SetTransform(const glm::mat4& newTransform);
	
	void Initialize(const char* requestedResource);

protected:
	glm::mat4 transform;
	
	// To be defined by render library
	void TransformUpdated();
	NodeResourceReference* resource;
};
}  // namespace Graphics
