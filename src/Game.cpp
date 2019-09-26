#include "Game.hpp"

#include <iostream>

#include <map>

#include "PhysicsVehicle.hpp"
#include "PhysicsWorld.hpp"

PhysicsState::PhysicsState() : vehicle(physicsWorld)
{
}

PhysicsState* gPhysicsState = nullptr;

void InitializeGame()
{
	gPhysicsState = new PhysicsState();
}

void UpdateGame(float frameTime)
{
	// processInput(input, vehicle);

	gPhysicsState->vehicle.Update(frameTime);
	gPhysicsState->physicsWorld.Update(frameTime);

	// // Use vehicle transform to position camera
	// if (useChaseCam)
	// {
	// 	const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
	// 	btTransform camTransform = vehicleTransform.inverse();
	// 	btScalar vehicleMat[16];
	// 	// vehicleTransform.getOpenGLMatrix(vehicleMat);
	// 	camTransform.getOpenGLMatrix(vehicleMat);

	// 	cam.ChaseCamera(vehicleMat);
	// }

	// physicsWorld.DebugRender();
}
