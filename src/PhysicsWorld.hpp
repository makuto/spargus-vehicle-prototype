#pragma once

// Much of the bullet code is copied from the Forklift demo for a good starting point

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

// extern bool bulletUseMCLPSolver;

#define rightAxisIndex 0
#define upAxisIndex 1
#define forwardAxisIndex 2

struct PhysicsWorld
{
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;

	btBroadphaseInterface* overlappingPairCache;
	btConstraintSolver* constraintSolver;

	btDiscreteDynamicsWorld* world;

	PhysicsWorld();
	void Update(float deltaTime);
	btRigidBody* localCreateRigidBody(btScalar mass, const btTransform& startTransform,
	                                  btCollisionShape* shape);
};
