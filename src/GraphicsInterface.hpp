#pragma once

#include <glm/mat4x4.hpp>         // mat4
#include "graphics/graphics.hpp"

namespace Graphics
{
// Implemented by RenderLibs
void OnWindowResized(int width, int height);
void Initialize(window& win, int width, int height);
void Update(float frameTime);
void Destroy();

// Change the render viewport position and size
// Note that because resizing the viewport is an expensive operation, the RenderLib should only
// resize buffers if it detects a change in width or height. Positioning the viewport is fine
void SetViewport(int x, int y, int width, int height);

// Camera
void SetCameraTransform(const glm::mat4& newTransform);
glm::mat4 GetCameraTransformCopy();
glm::mat4 GetCameraProjectionMatrixCopy();
}  // namespace Graphics
