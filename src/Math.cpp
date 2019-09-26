// This needs to happen only once
#define GB_MATH_IMPLEMENTATION
// This header has gb math included
#include "Math.hpp"

#include "btBulletDynamicsCommon.h"

void PositionRotationFromBulletTransform(const btTransform& transform, float* vPositionOut,
                                         float* qRotationOut)
{
	const btVector3& position = transform.getOrigin();
	for (int i = 0; i < 3; ++i)
		vPositionOut[i] = position[i];

	btQuaternion rotation = transform.getRotation();
	for (int i = 0; i < 4; ++i)
		qRotationOut[i] = rotation[i];
}
