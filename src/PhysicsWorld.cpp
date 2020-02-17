#include "PhysicsWorld.hpp"

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include <algorithm>

#include "Logging.hpp"
#include "Performance.hpp"

void SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep);

// TODO: Find out what this means
bool bulletUseMCLPSolver = true;

PhysicsWorld::PhysicsWorld()
{
	PerfTimeNamedScope(physicsWorldInit, "Physics World constructor", tracy::Color::DeepPink);

	// btVector3 groundExtents(50, 50, 50);
	// groundExtents[upAxis] = 3;
	// btCollisionShape* groundShape = new btBoxShape(groundExtents);
	// m_collisionShapes.push_back(groundShape);
	collisionConfiguration = new btDefaultCollisionConfiguration();
	collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
	btVector3 worldMin(-5000.0, -5000.0, -5000.0);
	btVector3 worldMax(5000.0, 5000.0, 5000.0);
	overlappingPairCache = new btAxisSweep3(worldMin, worldMax);
	if (bulletUseMCLPSolver)
	{
		btDantzigSolver* mlcp = new btDantzigSolver();
		// btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel;
		btMLCPSolver* sol = new btMLCPSolver(mlcp);
		constraintSolver = sol;
	}
	else
	{
		constraintSolver = new btSequentialImpulseConstraintSolver();
	}
	world = new btDiscreteDynamicsWorld(collisionDispatcher, overlappingPairCache, constraintSolver,
	                                    collisionConfiguration);

	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	// debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawContactPoints |
	                         // btIDebugDraw::DBG_DrawNormals);

	world->setDebugDrawer(&debugDrawer);

	world->setInternalTickCallback(SimulationTickCallback);

	if (bulletUseMCLPSolver)
	{
		// for direct solver it is better to have a small A matrix
		world->getSolverInfo().m_minimumSolverBatchSize = 1;
	}
	else
	{
		// for direct solver, it is better to solve multiple objects together, small batches
		// have high overhead
		world->getSolverInfo().m_minimumSolverBatchSize = 128;
	}
	world->getSolverInfo().m_globalCfm = 0.00001;

	// world->setGravity(btVector3(0,0,0));

	// Ground (for testing only)
	if (false)
	{
		btVector3 groundExtents(100.0, 100.0, 100.0);
		groundExtents[upAxisIndex] = 3;
		btCollisionShape* groundShape = new btBoxShape(groundExtents);
		// TODO Don't leak this
		// collisionShapes.push_back(groundShape);

		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(btVector3(0, -3, 0));

		localCreateRigidBody(StaticRigidBodyMass, tr, groundShape);
	}

	// Scale sync with Adult Male Reference from assets/ScaleReference.blend
	if (false)
	{
		float height = 1.799f;
		btVector3 scaleReferenceExtents(0.249f / 2.f, height / 2.f, 0.55f / 2.f);
		// scaleReferenceExtents[upAxisIndex] = 3;
		btCollisionShape* shape = new btBoxShape(scaleReferenceExtents);
		// TODO Don't leak this
		// collisionShapes.push_back(shape);

		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(btVector3(0.f, height / 2.f, 0.f));

		localCreateRigidBody(StaticRigidBodyMass, tr, shape);
	}

	world->setWorldUserInfo(this);
}

void PhysicsWorld::Update(float deltaTime)
{
	float dt = deltaTime;

	if (world)
	{
		world->updateAabbs();

		// during idle mode, just run 1 simulation step maximum
		int maxSimSubSteps = 2;

		int numSimSteps;
		numSimSteps = world->stepSimulation(dt, maxSimSubSteps);

		if (world->getConstraintSolver()->getSolverType() == BT_MLCP_SOLVER)
		{
			btMLCPSolver* sol = (btMLCPSolver*)world->getConstraintSolver();
			int numFallbacks = sol->getNumFallbacks();
			if (numFallbacks)
			{
				static int totalFailures = 0;
				totalFailures += numFallbacks;
				LOGV << "MLCP solver failed " << totalFailures
				     << " times, falling back to btSequentialImpulseSolver (SI)";
			}
			sol->setNumFallbacks(0);
		}

// #define VERBOSE_FEEDBACK
#ifdef VERBOSE_FEEDBACK
		if (!numSimSteps)
			LOGV << "Interpolated transforms";
		else
		{
			if (numSimSteps > maxSimSubSteps)
			{
				// detect dropping frames
				LOGV << "Dropped (" << numSimSteps - maxSimSubSteps << ") simulation steps out of "
				     << numSimSteps;
			}
			else
			{
				LOGV << "Simulated (" << numSimSteps << ") steps";
			}
		}
#endif  // VERBOSE_FEEDBACK
	}
}

void PhysicsWorld::DebugRender()
{
	world->debugDrawWorld();
}

btRigidBody* PhysicsWorld::localCreateRigidBody(btScalar mass, const btTransform& startTransform,
                                                btCollisionShape* shape)
{
	PerfTimeNamedScope(worldCreateRigidBodyScope, "World add rigid body", tracy::Color::HotPink);

	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	// rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

		// using motionstate is recommended, it provides interpolation capabilities, and only
		// synchronizes 'active' objects
#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, myMotionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	// body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif  //

	world->addRigidBody(body);
	return body;
}

void PhysicsWorld::AddCollisionListener(CollisionListener listener)
{
	collisionListeners.push_back(listener);
}

// From https://github.com/MikeMcShaffry/gamecode4
// This is modified.
// Under LGPL 3.0 - Spargus is MIT, so by extension LGPL should be met due to MIT source available?
void SimulationTickCallback(btDynamicsWorld* const world, btScalar const timeStep)
{
	PerfTimeNamedScope(simulationCallbackScope, "World simulation callback",
	                   tracy::Color::LightSalmon);

	CollisionPairs currentTickCollisionPairs;

	PhysicsWorld* const physicsWorld = static_cast<PhysicsWorld*>(world->getWorldUserInfo());

	// look at all existing contacts
	btDispatcher* const dispatcher = world->getDispatcher();
	for (int manifoldIdx = 0; manifoldIdx < dispatcher->getNumManifolds(); ++manifoldIdx)
	{
		// get the "manifold", which is the set of data corresponding to a contact point
		//   between two physics objects
		btPersistentManifold const* const manifold =
		    dispatcher->getManifoldByIndexInternal(manifoldIdx);

		// get the two bodies used in the manifold.  Bullet stores them as void*, so we must cast
		//  them back to btRigidBody*s.  Manipulating void* pointers is usually a bad
		//  idea, but we have to work with the environment that we're given.  We know this
		//  is safe because we only ever add btRigidBodys to the simulation
		btRigidBody const* const body0 = static_cast<btRigidBody const*>(manifold->getBody0());
		btRigidBody const* const body1 = static_cast<btRigidBody const*>(manifold->getBody1());

		void* aUserPointer = body0->getUserPointer();
		void* bUserPointer = body1->getUserPointer();

		// always create the pair in a predictable order
		bool const swapped = body0 > body1;

		btRigidBody const* const sortedBodyA = swapped ? body1 : body0;
		btRigidBody const* const sortedBodyB = swapped ? body0 : body1;

		CollisionPair const thisPair = std::make_pair(sortedBodyA, sortedBodyB);
		currentTickCollisionPairs.insert(thisPair);

		if (physicsWorld->previousTickCollisionPairs.find(thisPair) ==
		    physicsWorld->previousTickCollisionPairs.end())
		{
			// this is a new contact, which wasn't in our list before.  send an event to the game.
			// physicsWorld->SendCollisionPairAddEvent(manifold, body0, body1);
			LOGD << "Now colliding: " << (void*)body0 << " " << (void*)body1;
			LOGD << "\tUser pointers: " << (void*)aUserPointer << " " << (void*)bUserPointer;

			for (const CollisionListener& listener : physicsWorld->collisionListeners)
				listener(body0, body1, CollisionState::NowColliding);
		}
	}

	CollisionPairs removedCollisionPairs;

	// use the STL set difference function to find collision pairs that existed during the previous
	// tick but not any more
	std::set_difference(physicsWorld->previousTickCollisionPairs.begin(),
	                    physicsWorld->previousTickCollisionPairs.end(),
	                    currentTickCollisionPairs.begin(), currentTickCollisionPairs.end(),
	                    std::inserter(removedCollisionPairs, removedCollisionPairs.begin()));

	for (CollisionPairs::const_iterator it = removedCollisionPairs.begin(),
	                                    end = removedCollisionPairs.end();
	     it != end; ++it)
	{
		btRigidBody const* const body0 = it->first;
		btRigidBody const* const body1 = it->second;

		LOGD << "No longer colliding: " << (void*)body0 << " " << (void*)body1;

		for (const CollisionListener& listener : physicsWorld->collisionListeners)
			listener(body0, body1, CollisionState::Separating);
	}

	// the current tick becomes the previous tick.  this is the way of all things.
	physicsWorld->previousTickCollisionPairs = currentTickCollisionPairs;
}
