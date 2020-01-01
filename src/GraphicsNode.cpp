#include "GraphicsNode.hpp"

namespace Graphics
{
const glm::mat4& Node::GetTransformConstRef() const
{
	return transform;
}

glm::mat4 Node::GetTransformCopy() const
{
	return transform;
}

void Node::SetTransform(const glm::mat4& newTransform)
{
	transform = newTransform;
	TransformUpdated();
}

// Stub implementation: to be implemented by graphics library
// 	void Node::Initialize(const char* requestedResource)
// 	{
// 	}
}  // namespace Graphics
