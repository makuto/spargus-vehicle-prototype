#include "GraphicsObject.hpp"

namespace Graphics
{
Object::Object() : transform(1.f)
{
}

const glm::mat4& Object::GetTransformConstRef() const
{
	return transform;
}

glm::mat4 Object::GetTransformCopy() const
{
	return transform;
}

void Object::SetTransform(const glm::mat4& newTransform)
{
	transform = newTransform;
	TransformUpdated();
}
}  // namespace Graphics
