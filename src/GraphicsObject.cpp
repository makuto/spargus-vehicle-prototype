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

glm::vec3 Object::GetPosition() const
{
	return transform[3];
}

}  // namespace Graphics
