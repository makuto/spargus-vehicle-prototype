#include "PhysicsVehicle.hpp"

#include "PhysicsWorld.hpp"

#include "Audio.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Utilities.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  //radians
#include <glm/vec3.hpp>           // vec3

#include <algorithm>
#include <vector>
#include <iostream>

// TODO This is messy
std::mutex g_playerVehiclesMutex;
static std::vector<PhysicsVehicle*> g_playerVehicles;

////////////////////////////////////////////////////////////////////////////////
// Vehicle constants
//

const btVector3 wheelDirectionCS0(0, -1, 0);
const btVector3 wheelAxleCS(-1, 0, 0);

////////////////////////////////////////////////////////////////////////////////
// Vehicle implementation
//

PhysicsVehicle::PhysicsVehicle(PhysicsWorld& physicsWorld) : ownerWorld(physicsWorld)
{
	float chassisWidthHalfExtents = chassisWidth / 2.f;
	float chassisLengthHalfExtents = chassisLength / 2.f;

	// Create chassis
	{
		btCollisionShape* chassisShape = new btBoxShape(
		    btVector3(chassisWidthHalfExtents, chassisHeight / 2, chassisLengthHalfExtents));
		collisionShapes.push_back(chassisShape);

		// Make it possible to go from the chassis shape to the vehicle
		shapeReference.type = CollisionShapeOwnerType::Vehicle;
		shapeReference.shapeCreator = this;
		chassisShape->setUserPointer(&shapeReference);

		btCompoundShape* compound = new btCompoundShape();
		collisionShapes.push_back(compound);
		btTransform localTransform;
		localTransform.setIdentity();
		// TODO: Figure this out
		// localTransform effectively shifts the center of mass with respect to the chassis
		localTransform.setOrigin(btVector3(0, 1, 0));

		compound->addChildShape(localTransform, chassisShape);

		// This was the lift or something for the forklift reference. Commented for future ref
		// {
		// 	btCollisionShape* suppShape = new btBoxShape(btVector3(0.5f, 0.1f, 0.5f));
		// 	btTransform suppLocalTrans;
		// 	suppLocalTrans.setIdentity();
		// 	// localTransform effectively shifts the center of mass with respect to the chassis
		// 	suppLocalTrans.setOrigin(btVector3(0, 1.0, 2.5));
		// 	compound->addChildShape(suppLocalTrans, suppShape);
		// }

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(0, 0, 0));

		carChassis = physicsWorld.localCreateRigidBody(massKg, transform, compound);
		// carChassis->setDamping(0.2,0.2);

		/// never deactivate the vehicle
		carChassis->setActivationState(DISABLE_DEACTIVATION);
	}

	vehicleRayCaster = new btDefaultVehicleRaycaster(physicsWorld.world);
	vehicle = new btRaycastVehicle(tuning, carChassis, vehicleRayCaster);

	physicsWorld.world->addVehicle(vehicle);

	// Create wheels
	{
		bool isFrontWheel = true;

		// choose coordinate system
		vehicle->setCoordinateSystem(rightAxisIndex, upAxisIndex, forwardAxisIndex);

		btVector3 connectionPointCS0(chassisWidthHalfExtents - (0.3 * wheelWidth), connectionHeight,
		                             chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
		                  wheelRadius, tuning, isFrontWheel);

		connectionPointCS0 = btVector3(-chassisWidthHalfExtents + (0.3 * wheelWidth),
		                               connectionHeight, chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
		                  wheelRadius, tuning, isFrontWheel);

		isFrontWheel = false;
		connectionPointCS0 = btVector3(chassisWidthHalfExtents - (0.3 * wheelWidth),
		                               connectionHeight, -chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
		                  wheelRadius, tuning, isFrontWheel);

		connectionPointCS0 = btVector3(-chassisWidthHalfExtents + (0.3 * wheelWidth),
		                               connectionHeight, -chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
		                  wheelRadius, tuning, isFrontWheel);

		for (int i = 0; i < vehicle->getNumWheels(); i++)
		{
			btWheelInfo& wheel = vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = suspensionDamping;
			wheel.m_wheelsDampingCompression = suspensionCompression;
			wheel.m_frictionSlip = wheelFriction;
			wheel.m_rollInfluence = rollInfluence;
		}
	}

	Reset();

	// Initialize gearbox
	{
		// Gear 0 = neutral
		gearboxRatios.push_back(0.f);
		gearboxRatios.push_back(15.f);
		gearboxRatios.push_back(10.f);
		gearboxRatios.push_back(7.f);
		gearboxRatios.push_back(5.f);

		numGears = gearboxRatios.size();
	}

	// Initialize graphics
	chassisRender.Initialize("BasicBuggy_Chassis");
	wheelRender.resize(vehicle->getNumWheels());
	for (int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		Graphics::Object& wheelNode = wheelRender[i];
		// Disable front wheel for now, because the model isn't ready
		wheelNode.Initialize(i < 2 && false ? "Wheel_Front" : "Wheel_Rear");
	}

	{
		const std::lock_guard<std::mutex> lock(g_playerVehiclesMutex);
		g_playerVehicles.push_back(this);
	}
}

PhysicsVehicle::~PhysicsVehicle()
{
	const std::lock_guard<std::mutex> lock(g_playerVehiclesMutex);
	g_playerVehicles.erase(std::remove(g_playerVehicles.begin(), g_playerVehicles.end(), this),
	                       g_playerVehicles.end());
}

void PhysicsVehicle::Reset()
{
	VehicleSteering = 0.f;
	BrakingForce = defaultBrakingForce;
	ThrottlePercent = 0.f;

	carChassis->setCenterOfMassTransform(btTransform::getIdentity());
	carChassis->setLinearVelocity(btVector3(0, 0, 0));
	carChassis->setAngularVelocity(btVector3(0, 0, 0));
	ownerWorld.world->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(
	    carChassis->getBroadphaseHandle(), ownerWorld.world->getDispatcher());
	if (vehicle)
	{
		vehicle->resetSuspension();
		for (int i = 0; i < vehicle->getNumWheels(); i++)
		{
			// synchronize the wheels with the (interpolated) chassis worldtransform
			vehicle->updateWheelTransform(i, true);
		}
	}

	// Macoy: forklift stuff. Commented out, but may be useful later
	// btTransform liftTrans;
	// liftTrans.setIdentity();
	// liftTrans.setOrigin(m_liftStartPos);
	// liftBody->activate();
	// liftBody->setCenterOfMassTransform(liftTrans);
	// liftBody->setLinearVelocity(btVector3(0, 0, 0));
	// liftBody->setAngularVelocity(btVector3(0, 0, 0));

	// btTransform forkTrans;
	// forkTrans.setIdentity();
	// forkTrans.setOrigin(m_forkStartPos);
	// m_forkBody->activate();
	// m_forkBody->setCenterOfMassTransform(forkTrans);
	// m_forkBody->setLinearVelocity(btVector3(0, 0, 0));
	// m_forkBody->setAngularVelocity(btVector3(0, 0, 0));

	// //	m_liftHinge->setLimit(-LIFT_EPS, LIFT_EPS);
	// m_liftHinge->setLimit(0.0f, 0.0f);
	// m_liftHinge->enableAngularMotor(false, 0, 0);

	// m_forkSlider->setLowerLinLimit(0.1f);
	// m_forkSlider->setUpperLinLimit(0.1f);
	// m_forkSlider->setPoweredLinMotor(false);

	// btTransform loadTrans;
	// loadTrans.setIdentity();
	// loadTrans.setOrigin(m_loadStartPos);
	// m_loadBody->activate();
	// m_loadBody->setCenterOfMassTransform(loadTrans);
	// m_loadBody->setLinearVelocity(btVector3(0, 0, 0));
	// m_loadBody->setAngularVelocity(btVector3(0, 0, 0));
}

// The heavily simplified/layman drivetrain model
// I'm pulling much of this out of my ass for a good feel (and out of ignorance)
// Assume a transmission efficiency of 100% and an ideal differential (both wheels get the same
// amount of force regardless of conditions)
// See "references/A Vehicle Dynamics Model for Driving Simulators.pdf"
float PhysicsVehicle::EngineForceFromThrottle(float deltaTime, float throttlePercent,
                                              int selectedGear, float& engineRpmOut) const
{
	// Directly control output force
	if (simpleDrivetrain)
		return maxEngineForce * throttlePercent;

	float gearboxRatio = 1.f;
	if (selectedGear >= gearboxRatios.size())
		LOGE << "Gear " << selectedGear << " is not in range of gearbox (" << gearboxRatios.size()
		     << "gears)";
	else
		gearboxRatio = gearboxRatios[selectedGear];

	float engineRadiansPerStep = 0;

	// While clutch is engaged, engine speed comes from the speed of the wheels
	// TODO: Add case for if rear wheels are not contacting the ground
	if (ClutchPercent < 0.3f)
	{
		float maxDeltaRotation = -1000000.f;
		// Only do rear wheels, because this is a rear-wheel drive
		for (int i = 2; i < vehicle->getNumWheels(); i++)
		{
			const btWheelInfo& wheelInfo = vehicle->getWheelInfo(i);
			maxDeltaRotation = glm::max(wheelInfo.m_deltaRotation, maxDeltaRotation);
		}

		LOGD << "Rear wheel max Delta Rotation = " << maxDeltaRotation;

		// TODO This isn't actually per second yet
		engineRadiansPerStep = maxDeltaRotation * gearboxRatio;
	}

	// TODO Need to take the same step from the wheel update calculations to get the same result
	engineRpmOut =
	    (engineRadiansPerStep * (1 / deltaTime)) * (1.f / (2.f * glm::pi<float>())) * (60.f / 1.f);

	// Handle idle (manuals should stall I think? need to do some research on this)
	if (engineRpmOut < idleEngineRpm)
		engineRpmOut = idleEngineRpm;

	// Blown engine
	if (engineRpmOut > maxEngineRpm)
		engineRpmOut = maxEngineRpm;

	// TODO: Fix reverse always using 1st gear (limit reverse speed)
	engineRpmOut = glm::abs(engineRpmOut);

	// TODO: Consider engine speed
	float engineInput = throttlePercent;

	// TODO: Define these using polynomial equations based on engine speed
	// TODO: Handle zero negative torque force if wheels are going in reverse
	float minEngineTorque = 0.f;
	float maxEngineTorque = 250.f;
	// From engine speed and throttle position, determine torque via interpolation
	float engineTorque = glm::mix(minEngineTorque, maxEngineTorque, engineInput);

	float gearboxOutputForce = engineTorque * gearboxRatio;
	return gearboxOutputForce;
}

void PhysicsVehicle::Update(float deltaTime)
{
	float engineForce = 0.f;
	{
		// TODO Technically we need the lock only for setting here.. messy
		const std::lock_guard<std::mutex> lock(engineDetailsMutex);
		engineForce = EngineForceFromThrottle(deltaTime, ThrottlePercent, SelectedGear, engineRpm);
	}

	// Automatic transmission
	int gearBeforeAutoShift = SelectedGear;
	if (engineRpm < 800.f && SelectedGear > 1)
		SelectedGear--;
	else if (engineRpm > 1400.f)
		SelectedGear++;
	SelectedGear = glm::clamp<int>(SelectedGear, 0, gearboxRatios.size() - 1);
	if (SelectedGear != gearBeforeAutoShift)
	{
		LOGV << "Shifted from " << gearBeforeAutoShift << " to " << SelectedGear;
		playVehicleShifting();
	}

	LOGV << "Input throttle: " << ThrottlePercent << " Gear: " << SelectedGear
	     << " output force: " << engineForce;

	// Rear-wheel drive
	for (int wheelIndex = 2; wheelIndex < vehicle->getNumWheels(); ++wheelIndex)
	{
		vehicle->applyEngineForce(engineForce, wheelIndex);
		vehicle->setBrake(BrakingForce, wheelIndex);
	}

	// Steering
	for (int wheelIndex = 0; wheelIndex < 2; ++wheelIndex)
	{
		vehicle->setSteeringValue(VehicleSteering, wheelIndex);
	}

	// Chassis rendering
	chassisRender.SetTransform(GetTransform());

	// Wheel rendering
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		// synchronize the wheels with the (interpolated) chassis worldtransform
		vehicle->updateWheelTransform(i, true);

		btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
		float wheelGraphicsMatrix[16];
		BulletTransformToHordeMatrix(tr, wheelGraphicsMatrix);
		glm::mat4 wheelMatrix;
		openGlMatrixToGlmMat4(wheelGraphicsMatrix, wheelMatrix);
		// Rotate wheels 1 and 3 (right side) so hubcap faces outwards
		if (i % 2 != 0)
		{
			glm::vec3 rotateYAxis = {0.f, 1.f, 0.f};

			glm::mat4 rotateTireY = glm::rotate(glm::mat4(1.f), glm::radians(180.f), rotateYAxis);
			wheelMatrix = wheelMatrix * rotateTireY;
		}

		wheelRender[i].SetTransform(wheelMatrix);
	}
}

glm::mat4 PhysicsVehicle::GetTransform() const
{
	const btMotionState* motionState = carChassis->getMotionState();
	if (motionState)
	{
		btTransform chassisWorldTransform;
		motionState->getWorldTransform(chassisWorldTransform);
		return BulletTransformToGlmMat4(chassisWorldTransform);
	}
	else
	{
		const btTransform& vehicleTransform = vehicle->getChassisWorldTransform();
		return BulletTransformToGlmMat4(vehicleTransform);
	}
}

glm::vec3 PhysicsVehicle::GetPosition() const
{
	const btMotionState* motionState = carChassis->getMotionState();
	if (motionState)
	{
		btTransform chassisWorldTransform;
		motionState->getWorldTransform(chassisWorldTransform);
		return glm::vec3(chassisWorldTransform.getOrigin().getX(),
		                 chassisWorldTransform.getOrigin().getY(),
		                 chassisWorldTransform.getOrigin().getZ());
	}
	else
	{
		const btTransform& vehicleTransform = vehicle->getChassisWorldTransform();
		return glm::vec3(vehicleTransform.getOrigin().getX(), vehicleTransform.getOrigin().getY(),
		                 vehicleTransform.getOrigin().getZ());
	}
}

void PhysicsVehicle::ApplyTorque(const glm::vec3& torque)
{
	carChassis->applyTorque(glmVec3ToBulletVector(torque));
}

bool PhysicsVehicle::WheelsContactingSurface()
{
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		const btWheelInfo& wheelInfo = vehicle->getWheelInfo(i);
		if (wheelInfo.m_raycastInfo.m_isInContact > 0.1f)
			return true;
	}

	return false;
}

float GetPlayerVehicleEngineRpmThreadSafe()
{
	const std::lock_guard<std::mutex> lock(g_playerVehiclesMutex);

	if (!g_playerVehicles.empty())
	{
		PhysicsVehicle* playerVehicle = g_playerVehicles[0];
		const std::lock_guard<std::mutex> lock(playerVehicle->engineDetailsMutex);
		return playerVehicle->engineRpm;
	}
	return 0.f;
}
