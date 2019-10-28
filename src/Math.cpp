#include "Math.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

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
