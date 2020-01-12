#pragma once

#include "DebugDraw.hpp"

#include <set>
#include <vector>
#include <functional>

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
class btRigidBody;

typedef std::pair<btRigidBody const*, btRigidBody const*> CollisionPair;
typedef std::set<CollisionPair> CollisionPairs;

enum class CollisionState
{
	NowColliding,
	Separating
};

typedef std::function<void(const btRigidBody*, const btRigidBody*, CollisionState)>
    CollisionListener;

class PhysicsWorld
{
private:
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;

	btBroadphaseInterface* overlappingPairCache;
	btConstraintSolver* constraintSolver;

	BulletDebugDraw debugDrawer;


protected:
	friend void SimulationTickCallback(btDynamicsWorld* const world, btScalar const timeStep);
	CollisionPairs previousTickCollisionPairs;
	std::vector<CollisionListener> collisionListeners;
	
public:
	btDiscreteDynamicsWorld* world;

	PhysicsWorld();

	void Update(float deltaTime);
	void DebugRender();

	// rigidbody is dynamic if and only if mass is non zero, otherwise static
	static constexpr btScalar StaticRigidBodyMass = 0.f;

	btRigidBody* localCreateRigidBody(btScalar mass, const btTransform& startTransform,
	                                  btCollisionShape* shape);

	void AddCollisionListener(CollisionListener listener);
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
