#include "PhysicsVehicle.hpp"

#include "PhysicsWorld.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
// Vehicle constants
//

#define CUBE_HALF_EXTENTS 1

btVector3 wheelDirectionCS0(0, -1, 0);
btVector3 wheelAxleCS(-1, 0, 0);

/// btRaycastVehicle is the interface for the constraint that implements the raycast vehicle
/// notice that for higher-quality slow-moving vehicles, another approach might be better
/// implementing explicit hinged-wheel constraints with cylinder collision, rather then raycasts
float gEngineForce = 0.f;

float defaultBreakingForce = 10.f;
float gBreakingForce = 100.f;

float maxEngineForce = 1000.f;  // this should be engine/velocity dependent
float maxBreakingForce = 100.f;

float gVehicleSteering = 0.f;
float steeringIncrement = 0.04f;
float steeringClamp = 0.3f;

float wheelRadius = 0.5f;
float wheelWidth = 0.4f;
float wheelFriction = 1000;  // BT_LARGE_FLOAT;

float suspensionStiffness = 20.f;
float suspensionDamping = 2.3f;
float suspensionCompression = 4.4f;
float rollInfluence = 0.1f;  // 1.0f;
btScalar suspensionRestLength(0.6);

////////////////////////////////////////////////////////////////////////////////
// Vehicle implementation
//

PhysicsVehicle::PhysicsVehicle(PhysicsWorld& physicsWorld) : ownerWorld(physicsWorld)
{
	// Create chassis
	{
		btCollisionShape* chassisShape = new btBoxShape(btVector3(1.f, 0.5f, 2.f));
		collisionShapes.push_back(chassisShape);

		btCompoundShape* compound = new btCompoundShape();
		collisionShapes.push_back(compound);
		btTransform localTrans;
		localTrans.setIdentity();
		// localTrans effectively shifts the center of mass with respect to the chassis
		localTrans.setOrigin(btVector3(0, 1, 0));

		compound->addChildShape(localTrans, chassisShape);

		{
			btCollisionShape* suppShape = new btBoxShape(btVector3(0.5f, 0.1f, 0.5f));
			btTransform suppLocalTrans;
			suppLocalTrans.setIdentity();
			// localTrans effectively shifts the center of mass with respect to the chassis
			suppLocalTrans.setOrigin(btVector3(0, 1.0, 2.5));
			compound->addChildShape(suppLocalTrans, suppShape);
		}

		btTransform transform;
		transform.setIdentity();
		// transform.setOrigin(btVector3(0, -3, 0));
		transform.setOrigin(btVector3(0, 0.f, 0));

		carChassis = physicsWorld.localCreateRigidBody(800, transform, compound);
		// carChassis->setDamping(0.2,0.2);

		/// never deactivate the vehicle
		carChassis->setActivationState(DISABLE_DEACTIVATION);
	}

	vehicleRayCaster = new btDefaultVehicleRaycaster(physicsWorld.world);
	vehicle = new btRaycastVehicle(tuning, carChassis, vehicleRayCaster);

	physicsWorld.world->addVehicle(vehicle);

	float connectionHeight = 1.2f;

	bool isFrontWheel = true;

	// choose coordinate system
	vehicle->setCoordinateSystem(rightAxisIndex, upAxisIndex, forwardAxisIndex);

	btVector3 connectionPointCS0(CUBE_HALF_EXTENTS - (0.3 * wheelWidth), connectionHeight,
	                             2 * CUBE_HALF_EXTENTS - wheelRadius);

	vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
	                  wheelRadius, tuning, isFrontWheel);
	connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS + (0.3 * wheelWidth), connectionHeight,
	                               2 * CUBE_HALF_EXTENTS - wheelRadius);

	vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
	                  wheelRadius, tuning, isFrontWheel);
	connectionPointCS0 = btVector3(-CUBE_HALF_EXTENTS + (0.3 * wheelWidth), connectionHeight,
	                               -2 * CUBE_HALF_EXTENTS + wheelRadius);
	isFrontWheel = false;
	vehicle->addWheel(connectionPointCS0, wheelDirectionCS0, wheelAxleCS, suspensionRestLength,
	                  wheelRadius, tuning, isFrontWheel);
	connectionPointCS0 = btVector3(CUBE_HALF_EXTENTS - (0.3 * wheelWidth), connectionHeight,
	                               -2 * CUBE_HALF_EXTENTS + wheelRadius);
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
	
	Reset();
}

void PhysicsVehicle::Reset()
{
	gVehicleSteering = 0.f;
	gBreakingForce = defaultBreakingForce;
	gEngineForce = 0.f;

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

void PhysicsVehicle::Update(float deltaTime)
{
	int wheelIndex = 2;
	vehicle->applyEngineForce(gEngineForce, wheelIndex);
	vehicle->setBrake(gBreakingForce, wheelIndex);
	wheelIndex = 3;
	vehicle->applyEngineForce(gEngineForce, wheelIndex);
	vehicle->setBrake(gBreakingForce, wheelIndex);

	wheelIndex = 0;
	vehicle->setSteeringValue(gVehicleSteering, wheelIndex);
	wheelIndex = 1;
	vehicle->setSteeringValue(gVehicleSteering, wheelIndex);

	const btVector3& carLinearVelocity = carChassis->getLinearVelocity();
	std::cout << "Vehicle linear velocity: " << carLinearVelocity.getX() << ", "
	          << carLinearVelocity.getY() << ", " << carLinearVelocity.getZ() << "\n";
}
