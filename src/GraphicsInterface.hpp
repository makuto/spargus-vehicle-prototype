#pragma once

#include <glm/mat4x4.hpp>         // mat4

namespace Graphics
{
// Implemented by RenderLibs
void OnWindowResized(int width, int height);
void Initialize(int width, int height);
void Update(float frameTime);
void Destroy();

void SetViewport(int x, int y, int width, int height);

// Camera
void SetCameraTransform(const glm::mat4& newTransform);
glm::mat4 GetCameraTransformCopy();
glm::mat4 GetCameraProjectionMatrixCopy();
}  // namespace Graphics
