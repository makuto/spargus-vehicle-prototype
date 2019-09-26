#pragma once

// Much of the bullet code is copied from the Forklift demo for a good starting point

// extern bool bulletUseMCLPSolver;

#define rightAxisIndex 0
#define upAxisIndex 1
#define forwardAxisIndex 2

#include "btBulletDynamicsCommon.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;

struct PhysicsWorld
{
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;

	btBroadphaseInterface* overlappingPairCache;
	btConstraintSolver* constraintSolver;

	btDiscreteDynamicsWorld* world;
	
	PhysicsWorld();

	void Update(float deltaTime);
	void DebugRender();

	btRigidBody* localCreateRigidBody(btScalar mass, const btTransform& startTransform,
	                                  btCollisionShape* shape);
};
