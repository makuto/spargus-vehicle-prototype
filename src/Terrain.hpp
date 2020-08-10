#pragma once

#include "btBulletDynamicsCommon.h"

#include <glm/vec3.hpp>

class PhysicsWorld;
void createCollisionHeightfield(PhysicsWorld& world, const glm::vec3& worldPosition);

// Important: Note that even if the type is called PHY_"FLOAT", this needs to be btScalar (i.e. can
// end up being a double)
typedef btScalar HeightfieldCell;

extern HeightfieldCell* g_rawHeightfieldData;
