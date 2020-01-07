#pragma once

#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>

//
// Constants
//
extern const glm::vec3 UpAxis; // Y
extern const glm::vec3 LeftAxis; // X
extern const glm::vec3 ForwardAxis; // Z

//
// Type conversions
//

// TODO add type safety
// typedef float HordeMatrix[16];

void openGlMatrixToGlmMat4(const float* openGlMatrix, glm::mat4& matOut);
void openGlMatrixToGlmMat4(const double* openGlMatrix, glm::mat4& matOut);
class btTransform;
void BulletTransformToHordeMatrix(const btTransform& transform, float* hordeMatrixOut);
glm::mat4 BulletTransformToGlmMat4(const btTransform& transform);
// Does not copy
inline float* glmMatrixToHordeMatrixRef(glm::mat4& mat)
{
	return &mat[0][0];
}

inline const float* glmMatrixToHordeMatrixRef(const glm::mat4& mat)
{
	return &mat[0][0];
}

class btVector3;
btVector3 glmVec3ToBulletVector(const glm::vec3& vec);

//
// Unit conversions
//
inline float KilometersToMiles(float kilometers)
{
	return kilometers / 1.609344f;
}
