#include "Math.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

const glm::vec3 UpAxis = {0.f, 1.f, 0.f};
const glm::vec3 LeftAxis = {1.f, 0.f, 0.f};
const glm::vec3 ForwardAxis = {0.f, 0.f, 1.f};

void openGlMatrixToGlmMat4(const float* openGlMatrix, glm::mat4& matOut)
{
	for (int i = 0; i < 16; i++)
	{
		matOut[i / 4][i % 4] = openGlMatrix[i];
	}
}

void openGlMatrixToGlmMat4(const double* openGlMatrix, glm::mat4& matOut)
{
	for (int i = 0; i < 16; i++)
	{
		matOut[i / 4][i % 4] = openGlMatrix[i];
	}
}

void BulletTransformToHordeMatrix(const btTransform& transform, float* hordeMatrixOut)
{
	btScalar bulletMat[16];
	transform.getOpenGLMatrix(bulletMat);
	for (size_t i = 0; i < sizeof(bulletMat) / sizeof(bulletMat[0]); i++)
	{
		hordeMatrixOut[i] = bulletMat[i];
	}
}

glm::mat4 BulletTransformToGlmMat4(const btTransform& transform)
{
	glm::mat4 convertedMatrix;

	btScalar bulletMat[16];
	transform.getOpenGLMatrix(bulletMat);
	openGlMatrixToGlmMat4(bulletMat, convertedMatrix);
	return convertedMatrix;
}

btVector3 glmVec3ToBulletVector(const glm::vec3& vec)
{
	btVector3 newVector(vec[0], vec[1], vec[2]);
	return newVector;
}


float interpolateRange(float startA, float endA, float startB, float endB, float bValue)
{
	float interpolateTo = (bValue - startB) / (endB - startB);
	return (interpolateTo * (endA - startA)) + startA;
}
