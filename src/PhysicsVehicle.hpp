#pragma once

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include "PhysicsWorld.hpp"
#include "GraphicsObject.hpp"

#include <glm/vec3.hpp>    // vec3

#include <vector>

class PhysicsWorld;

// TODO: Add destructor
class PhysicsVehicle
{
public:
	PhysicsVehicle(PhysicsWorld& physicsWorld);
	void Update(float deltaTime);

	void Reset();

	// Controls
	float EngineForce = 0.f;
	float BrakingForce = 100.f;
	float VehicleSteering = 0.f;

	btRaycastVehicle* vehicle;

	glm::vec3 GetPosition() const;
	glm::mat4 GetTransform() const;

	bool WheelsContactingSurface();

	void ApplyTorque(const glm::vec3& torque);
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
	
	// Used to get from a collision shape to this structure
	CollisionShapeOwnerReference shapeReference;

	// Rendering
	Graphics::Object chassisRender;
	std::vector<Graphics::Object> wheelRender;

	////////////////////////////////////////////////////////////////////////////////////////////////
	// Constants
	//

public:
	// Control constants
	float maxEngineForce = 1600.f;  // this should be engine/velocity dependent
	float maxBrakingForce = 400.f;

	float steeringIncrement = 0.04f;
	float steeringClamp = 0.3f;

	// I'm not sure why this needs to be so big...need to learn more about physics
	// Only multiply by sixty to compensate for adding framerate independence on numbers that I
	// liked before making it rate independent
	float airControlMaxPitchTorquePerSecond = 600.f * 60;
	float airControlMaxRollTorquePerSecond = 600.f * 60;

private:
	// I couldn't find any hard numbers, so let's guess around 1000lbs.
	// 1000lbs = ~453kg.
	float massKg = 453.f;

	// Example defaults
	// float chassisWidth = 1.f;
	// float chassisHeight = 0.5f;
	// float chassisLength = 2.f;

	// Double width of the chassis to approximate wheel collision, and increase balance
	float chassisWidth = 3.0f;
	// float chassisWidth = 1.418f;
	// Use the height at the middle of the chassis until we get nonrectangular collision set up
	float chassisHeight = 1.3725f;
	// float chassisHeight = 0.5f;
	float chassisLength = 4.12349f;

	float defaultBrakingForce = 20.f;

	// The axle height relative to the vehicle (I think)
	// float connectionHeight = 0.827f;  // 1.2f;
	float connectionHeight = 0.727f;  // 1.2f;
	// Use same radius wheels for now
	// TODO add support for different rear/front wheel sizes
	float wheelRadius = 1.64f / 2.f;
	// Tread width
	float wheelWidth = 0.5f;
	// Default by bullet
	// float wheelFriction = 1000;  // BT_LARGE_FLOAT;
	// Pretty slidey here, with some good characteristics (reverse 180s are fun)
	// float wheelFriction = 1.5f;  // BT_LARGE_FLOAT;
	float wheelFriction = 5.f;  // BT_LARGE_FLOAT;

	float suspensionStiffness = 20.f;
	float suspensionDamping = 2.3f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.1f;  // 1.0f;
	btScalar suspensionRestLength = 0.6;
};
