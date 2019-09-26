#pragma once

#include "PhysicsWorld.hpp"
#include "PhysicsVehicle.hpp"

struct PhysicsState
{
	PhysicsWorld physicsWorld;
	PhysicsVehicle vehicle;

	PhysicsState();
};

extern PhysicsState* gPhysicsState;

void InitializeGame();
void UpdateGame(float frameTime);
