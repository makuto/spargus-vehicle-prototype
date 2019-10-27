#pragma once

#include <glm/mat4x4.hpp>  // mat4

void openGlMatrixToGlmMat4(const float* openGlMatrix, glm::mat4& matOut);
void openGlMatrixToGlmMat4(const double* openGlMatrix, glm::mat4& matOut);
class btTransform;
void hordeMatrixFromBulletTransform(const btTransform& transform, float* hordeMatrixOut);
