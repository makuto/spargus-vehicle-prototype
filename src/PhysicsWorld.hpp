#pragma once

#include "DebugDraw.hpp"

// Much of the bullet code is copied from the Forklift demo for a good starting point

// extern bool bulletUseMCLPSolver;

#define rightAxisIndex 0
#define upAxisIndex 1
#define forwardAxisIndex 2

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
	
	BulletDebugDraw debugDrawer;

	PhysicsWorld();

	void Update(float deltaTime);
	void DebugRender();

	// rigidbody is dynamic if and only if mass is non zero, otherwise static
	static constexpr btScalar StaticRigidBodyMass = 0.f;

	btRigidBody* localCreateRigidBody(btScalar mass, const btTransform& startTransform,
	                                  btCollisionShape* shape);

	// void SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep);
};

enum class CollisionShapeOwnerType : int
{
	None = 0,
	Vehicle
};

struct CollisionShapeOwnerReference
{
	CollisionShapeOwnerType type;
	void* shapeCreator;
};
