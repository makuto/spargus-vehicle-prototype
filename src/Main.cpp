#include <iostream>

#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include "graphics/graphics.hpp"
#include "input/input.hpp"

// TODO: Find out what this means
bool bulletUseMCLPSolver = true;

// Window variables
int WindowWidth = 1200;
int WindowHeight = 700;
#define WIN_BACKGROUND_COLOR 34, 34, 34, 255

struct BulletWorld
{
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* collisionDispatcher;

	btBroadphaseInterface* overlappingPairCache;
	btConstraintSolver* constraintSolver;

	btDiscreteDynamicsWorld* world;

	BulletWorld()
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
		world = new btDiscreteDynamicsWorld(collisionDispatcher, overlappingPairCache,
		                                    constraintSolver, collisionConfiguration);

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
	}
};

void initializeWindow(window& win)
{
	win.setBackgroundColor(WIN_BACKGROUND_COLOR);

	// shouldClose manages some state
	win.shouldClose();
	win.update();

	win.shouldClear(true);
	win.getBase()->setVerticalSyncEnabled(true);
	win.getBase()->setFramerateLimit(30);
}

void windowResizeCB(float width, float height)
{
	WindowWidth = width;
	WindowHeight = height;
}

int main()
{
	std::cout << "Spargus Vehicle Prototype\n";

	BulletWorld physicsWorld;

	window mainWindow(WindowWidth, WindowHeight, "Spargus Vehicle Prototype", &windowResizeCB);
	initializeWindow(mainWindow);
	
	inputManager input(&mainWindow);
}
