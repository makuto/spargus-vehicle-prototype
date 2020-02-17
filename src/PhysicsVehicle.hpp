#pragma once

#include "BulletDynamics/Vehicle/btRaycastVehicle.h"

#include "PhysicsWorld.hpp"

#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>    // vec3

#include <mutex>
#include <thread>
#include <vector>

class PhysicsWorld;

#define VEHICLE_MAX_NUM_WHEELS 4

// TODO: Add destructor
class PhysicsVehicle
{
public:
	PhysicsVehicle(PhysicsWorld& physicsWorld);
	~PhysicsVehicle();
	void Update(float deltaTime);

	void Reset();

	// Controls
	// Percent means 0.f through 1.f
	float ThrottlePercent = 0.f;
	// Clutch at 0 = fully engaged. 1 = engine disconnected from wheels
	float ClutchPercent = 0.f;
	// Gear 0 = neutral
	int SelectedGear = 1;
	float BrakingForce = 100.f;
	float VehicleSteering = 0.f;

	// Directly tie throttle percent to max possible force output
	bool simpleDrivetrain = false;

	btRaycastVehicle* vehicle;

	// Note that these cache the last position since Update() was called. This is because the
	// btMotionState interpolation will happen for each call to the chassis GetTransform, which
	// means that calls throughout the frame would get different positions, leading to
	// synchronization issues
	glm::vec3 GetPosition() const;
	glm::mat4 GetTransform() const;
	glm::mat4 GetWheelTransform(int wheelIndex) const;

	bool WheelsContactingSurface();

	void ApplyTorque(const glm::vec3& torque);

	// Drivetrain
	float EngineForceFromThrottle(float deltaTime, float throttlePercent, int selectedGear,
	                              float& engineRpmOut) const;

	// Set in constructor. Don't change
	int numGears;

private:
	btRigidBody* carChassis;
	/// btRaycastVehicle is the interface for the constraint that implements the raycast vehicle
	/// notice that for higher-quality slow-moving vehicles, another approach might be better
	/// implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts
	btRaycastVehicle::btVehicleTuning tuning;
	btVehicleRaycaster* vehicleRayCaster;

	glm::mat4 lastUpdateTransform;
	glm::mat4 lastUpdateWheelTransforms[VEHICLE_MAX_NUM_WHEELS];

	PhysicsWorld& ownerWorld;

	// For cleanup only
	std::vector<btCollisionShape*> collisionShapes;

	// Used to get from a collision shape to this structure
	CollisionShapeOwnerReference shapeReference;

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
	float airControlMaxPitchTorquePerSecond = 600.f * 60.f;
	float airControlMaxRollTorquePerSecond = 600.f * 60.f;

	// TODO Make getter?
	float engineRpm;

	float idleEngineRpm = 300.f;
	float maxEngineRpm = 3000.f;

	// "Realistic"
	// float idleEngineRpm = 600.f;
	// float maxEngineRpm = 7500.f;
private:
	// I couldn't find any hard numbers, so let's guess around 1000lbs.
	// 1000lbs = ~453kg.
	float massKg = 453.f;

	// Example defaults
	// float chassisWidth = 1.f;
	// float chassisHeight = 0.5f;
	// float chassisLength = 2.f;

	// Double width of the chassis to approximate wheel collision, and increase balance
	// float chassisWidth = 1.17871f; // Actual
	float chassisWidth = 2.3029f; // Wheelbase to wheelbase
	// float chassisWidth = 1.418f;
	// Use the height at the middle of the chassis until we get nonrectangular collision set up
	// float chassisHeight = 1.029375f;
	float chassisHeight = 1.022;
	// float chassisHeight = 0.5f;
	float chassisLength = 3.37965f;

	// chassisLocalOffset shifts the chassis collision shape relative to the vehicle origin
	// If chassisLocalOffset = {0.f, 0.f, 0.f}, the chassis half-width, half-height, half-length
	// bounding box will be centered around 0, 0, 0
	const btVector3 chassisLocalOffset = {0.f, 1.022f / 2.f, 0.f};

	float defaultBrakingForce = 20.f;

	// Offset the top of the wheel relative to the origin
	// float connectionHeight = 0.827f;  // 1.2f;
	// float connectionHeight = 0.54525f;  // 1.2f;
	float connectionHeight = 0.7436f;  // 1.2f;
	// Use same radius wheels for now
	// TODO add support for different rear/front wheel sizes
	// float wheelRadius = 1.23f / 2.f;
	float wheelRadius = 0.7436f - 0.1227f;
	// Tread width
	float wheelWidth = 0.375f;
	// Default by bullet
	// float wheelFriction = 1000;  // BT_LARGE_FLOAT;
	// Pretty slidey here, with some good characteristics (reverse 180s are fun)
	// float wheelFriction = 1.5f;  // BT_LARGE_FLOAT;
	float wheelFriction = 5.f;  // BT_LARGE_FLOAT;

	// Not really necessary to customize these, unless you're making something weird
	// These need to be normalized, otherwise they scale the wheels
	const btVector3 wheelDirectionCS0 = {0.f, -1.f, 0.f};
	// The normal of the hubcap, basically
	const btVector3 wheelAxleCS = {-1.f, 0.f, 0.f};
	// const btVector3 wheelAxleCS = {0.f, 0.f, -1.f};

	float suspensionStiffness = 20.f;
	float suspensionDamping = 2.3f;
	float suspensionCompression = 4.4f;
	float rollInfluence = 0.1f;  // 1.0f;
	btScalar suspensionRestLength = 0.6f;

	std::vector<float> gearboxRatios;

	// See comment above GetTransform() on why you probably don't want this function
	// (Unless you're in the internals of Update())
	glm::vec3 GetInterpolatedPosition() const;
	glm::mat4 GetInterpolatedTransform() const;

protected:
	friend float GetPlayerVehicleEngineRpmThreadSafe();
	std::mutex engineDetailsMutex;
};

// TODO Think of a good way of doing this
float GetPlayerVehicleEngineRpmThreadSafe();

extern std::mutex g_vehiclesMutex;
extern std::vector<PhysicsVehicle*> g_vehicles;
