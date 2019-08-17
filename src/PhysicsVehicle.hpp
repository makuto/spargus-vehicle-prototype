#pragma once

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include <vector>

struct PhysicsWorld;

// TODO: Add destructor
struct PhysicsVehicle
{
	btRigidBody* carChassis;
	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster* vehicleRayCaster;
	btRaycastVehicle* vehicle;

	PhysicsWorld& ownerWorld;

	// For cleanup only
	std::vector<btCollisionShape*> collisionShapes;

	PhysicsVehicle(PhysicsWorld& physicsWorld);
	void Update(float deltaTime);
	
	void Reset();
};
