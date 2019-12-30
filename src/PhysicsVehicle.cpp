#include "PhysicsVehicle.hpp"

#include "PhysicsWorld.hpp"

#include "Logging.hpp"

// For wheels
#include <Horde3D.h>
#include "Math.hpp"
#include "Render_Horde3D.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  //radians
#include <glm/vec3.hpp>           // vec3

#include <iostream>

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
}

void PhysicsVehicle::Reset()
{
	VehicleSteering = 0.f;
	BrakingForce = defaultBrakingForce;
	EngineForce = 0.f;

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

	// Macoy: forklift stuff. Commented out.
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
	// Rear-wheel drive
	for (int wheelIndex = 2; wheelIndex < vehicle->getNumWheels(); ++wheelIndex)
	{
		vehicle->applyEngineForce(EngineForce, wheelIndex);
		vehicle->setBrake(BrakingForce, wheelIndex);
	}

	// Steering
	for (int wheelIndex = 0; wheelIndex < 2; ++wheelIndex)
	{
		vehicle->setSteeringValue(VehicleSteering, wheelIndex);
	}

	// Wheels
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		// synchronize the wheels with the (interpolated) chassis worldtransform
		vehicle->updateWheelTransform(i, true);

		if (buggyWheelNodes[0])
		{
			btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
			float wheelGraphicsMatrix[16];
			BulletTransformToHordeMatrix(tr, wheelGraphicsMatrix);
			glm::mat4 wheelMatrix;
			openGlMatrixToGlmMat4(wheelGraphicsMatrix, wheelMatrix);
			// Rotate wheels 1 and 3 (right side) so hubcap faces outwards
			if (i % 2 != 0)
			{
				glm::vec3 rotateYAxis = {0.f, 1.f, 0.f};

				glm::mat4 rotateTireY =
				    glm::rotate(glm::mat4(1.f), glm::radians(180.f), rotateYAxis);
				wheelMatrix = wheelMatrix * rotateTireY;
			}
			h3dSetNodeTransMat(buggyWheelNodes[i], glmMatrixToHordeMatrixRef(wheelMatrix));
		}
	}

	// Debug prints
	if (false)
	{
		// const btVector3& carLinearVelocity = carChassis->getLinearVelocity();
		// std::cout << "Vehicle linear velocity: " << carLinearVelocity.getX() << ", "
		// << carLinearVelocity.getY() << ", " << carLinearVelocity.getZ() << "\n";
		const btTransform& vehicleTransform = vehicle->getChassisWorldTransform();
		LOGD << "Vehicle location: " << vehicleTransform.getOrigin().getX() << ", "
		     << vehicleTransform.getOrigin().getY() << ", " << vehicleTransform.getOrigin().getZ();
	}
}

glm::mat4 PhysicsVehicle::GetTransform() const
{
	const btTransform& vehicleTransform = vehicle->getChassisWorldTransform();
	return BulletTransformToGlmMat4(vehicleTransform);
}

glm::vec3 PhysicsVehicle::GetPosition() const
{
	const btTransform& vehicleTransform = vehicle->getChassisWorldTransform();
	return glm::vec3(vehicleTransform.getOrigin().getX(), vehicleTransform.getOrigin().getY(),
	                 vehicleTransform.getOrigin().getZ());
}
