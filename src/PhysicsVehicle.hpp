#pragma once

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include <vector>

struct PhysicsWorld;

// TODO: Add destructor
struct PhysicsVehicle
{
	PhysicsVehicle(PhysicsWorld& physicsWorld);
	void Update(float deltaTime);

	void Reset();

	// Controls
	float EngineForce = 0.f;
	float BrakingForce = 100.f;
	float VehicleSteering = 0.f;

	// Control constants
	float maxEngineForce = 1000.f;  // this should be engine/velocity dependent
	float maxBrakingForce = 100.f;

	float steeringIncrement = 0.04f;
	float steeringClamp = 0.3f;

	btRaycastVehicle* vehicle;

private:
	btRigidBody* carChassis;
	/// btRaycastVehicle is the interface for the constraint that implements the raycast vehicle
	/// notice that for higher-quality slow-moving vehicles, another approach might be better
	/// implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts
	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster* vehicleRayCaster;

	PhysicsWorld& ownerWorld;

	// For cleanup only
	std::vector<btCollisionShape*> collisionShapes;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constants
	//
	float defaultBrakingForce = 10.f;

	float wheelRadius = 0.5f;
	float wheelWidth = 0.4f;
	float wheelFriction = 1000;  // BT_LARGE_FLOAT;

	float suspensionStiffness = 20.f;
	float suspensionDamping = 2.3f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.1f;  // 1.0f;
	btScalar suspensionRestLength = 0.6;
};
