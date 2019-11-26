#include "PhysicsWorld.hpp"

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include <iostream>

void SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep);

// TODO: Find out what this means
bool bulletUseMCLPSolver = true;

PhysicsWorld::PhysicsWorld()
{
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

	// debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);
	debugDrawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe  //|
	                         // btIDebugDraw::DBG_DrawAabb |
	                         // btIDebugDraw::DBG_DrawContactPoints
	                         // btIDebugDraw::DBG_DrawNormals
	);

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
				std::cout << "MLCP solver failed " << totalFailures
				          << " times, falling back to btSequentialImpulseSolver (SI)\n";
			}
			sol->setNumFallbacks(0);
		}

// #define VERBOSE_FEEDBACK
#ifdef VERBOSE_FEEDBACK
		if (!numSimSteps)
			std::cout << "Interpolated transforms\n";
		else
		{
			if (numSimSteps > maxSimSubSteps)
			{
				// detect dropping frames
				std::cout << "Dropped (" << numSimSteps - maxSimSubSteps
				          << ") simulation steps out of " << numSimSteps << "\n";
			}
			else
			{
				std::cout << "Simulated (" << numSimSteps << ") steps\n";
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

void SimulationTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
	int numManifolds = world->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		const btPersistentManifold* contactManifold =
		    world->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA =
		    static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* obB =
		    static_cast<const btCollisionObject*>(contactManifold->getBody1());
		
		const btCollisionShape* aShape = obA->getCollisionShape();

		void* aUserPointer = aShape->getUserPointer();
		void* bUserPointer = obB->getUserPointer();
		if (aUserPointer)
		{
			const CollisionShapeOwnerReference* aShapeOwnerReference =
			    static_cast<const CollisionShapeOwnerReference*>(aUserPointer);
			std::cout << aShapeOwnerReference->shapeCreator << "\n";
		}
		if (bUserPointer)
		{
			const CollisionShapeOwnerReference* bShapeOwnerReference =
			    static_cast<const CollisionShapeOwnerReference*>(bUserPointer);
			std::cout << bShapeOwnerReference->shapeCreator << "\n";
		}
		// std::cout << "Collision!\n";
	}
}
