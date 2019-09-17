#include "PhysicsWorld.hpp"

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include <iostream>

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
	btVector3 worldMin(-1000, -1000, -1000);
	btVector3 worldMax(1000, 1000, 1000);
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
	world->setDebugDrawer(&debugDrawer);

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
	
	//world->setGravity(btVector3(0,0,0));	

	// Ground (for testing only)
	{
		btVector3 groundExtents(100.0, 100.0, 100.0);
		groundExtents[upAxisIndex] = 3;
		btCollisionShape* groundShape = new btBoxShape(groundExtents);
		// TODO Don't leak this
		// collisionShapes.push_back(groundShape);

		btTransform tr;
		tr.setIdentity();
		tr.setOrigin(btVector3(0, -3, 0));

		localCreateRigidBody(0, tr, groundShape);
	}
}

void PhysicsWorld::Update(float deltaTime)
{
	float dt = deltaTime;

	if (world)
	{
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
