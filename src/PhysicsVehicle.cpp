#include "PhysicsVehicle.hpp"

#include "PhysicsWorld.hpp"

#include "Audio.hpp"
#include "Color.hpp"
#include "DebugDraw.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Performance.hpp"
#include "Utilities.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>  // angle
#include <glm/mat4x4.hpp>            // mat4
#include <glm/trigonometric.hpp>     //radians
#include <glm/vec3.hpp>              // vec3

#include "BulletDynamics/ConstraintSolver/btContactConstraint.h"
#include "BulletDynamics/Vehicle/btWheelInfo.h"

#include <algorithm>
#include <iostream>
#include <vector>

// TODO This is messy
std::mutex g_vehiclesMutex;
std::vector<PhysicsVehicle*> g_vehicles;

////////////////////////////////////////////////////////////////////////////////
// Vehicle implementation
//

static btScalar sideFrictionStiffness2 = btScalar(1.0f);

CustomRaycastVehicle::CustomRaycastVehicle(const btVehicleTuning& tuning, btRigidBody* chassis,
                                           btVehicleRaycaster* raycaster)
    : btRaycastVehicle(tuning, chassis, raycaster)
{
}

// We need to make friction a factor of velocity
void CustomRaycastVehicle::updateFriction(btScalar timeStep)
{
	bool hackySlide = false;

	// calculate the impulse, so that the wheels don't move sidewards
	int numWheel = getNumWheels();
	if (!numWheel)
		return;

	m_forwardWS.resize(numWheel);
	m_axle.resize(numWheel);
	m_forwardImpulse.resize(numWheel);
	m_sideImpulse.resize(numWheel);

	int numWheelsOnGround = 0;

	// collapse all those loops into one!
	for (int i = 0; i < getNumWheels(); i++)
	{
		btWheelInfo& wheelInfo = m_wheelInfo[i];
		class btRigidBody* groundObject =
		    (class btRigidBody*)wheelInfo.m_raycastInfo.m_groundObject;
		if (groundObject)
			numWheelsOnGround++;
		m_sideImpulse[i] = btScalar(0.);
		m_forwardImpulse[i] = btScalar(0.);
	}

	{
		for (int i = 0; i < getNumWheels(); i++)
		{
			btWheelInfo& wheelInfo = m_wheelInfo[i];

			class btRigidBody* groundObject =
			    (class btRigidBody*)wheelInfo.m_raycastInfo.m_groundObject;

			if (groundObject)
			{
				const btTransform& wheelTrans = getWheelTransformWS(i);

				btMatrix3x3 wheelBasis0 = wheelTrans.getBasis();
				m_axle[i] =
				    -btVector3(wheelBasis0[0][m_indexRightAxis], wheelBasis0[1][m_indexRightAxis],
				               wheelBasis0[2][m_indexRightAxis]);

				const btVector3& surfNormalWS = wheelInfo.m_raycastInfo.m_contactNormalWS;
				btScalar proj = m_axle[i].dot(surfNormalWS);
				m_axle[i] -= surfNormalWS * proj;
				m_axle[i] = m_axle[i].normalize();

				m_forwardWS[i] = surfNormalWS.cross(m_axle[i]);
				m_forwardWS[i].normalize();

				resolveSingleBilateral(*m_chassisBody, wheelInfo.m_raycastInfo.m_contactPointWS,
				                       *groundObject, wheelInfo.m_raycastInfo.m_contactPointWS,
				                       btScalar(0.), m_axle[i], m_sideImpulse[i], timeStep);

				m_sideImpulse[i] *= sideFrictionStiffness2;
			}
		}
	}

	btScalar sideFactor = btScalar(1.);
	btScalar fwdFactor = 0.5;

	glm::vec3 vehicleVelocity = BulletVectorToGlmVec3(m_chassisBody->getLinearVelocity());
	glm::vec3 vehicleNormalVelocity = glm::normalize(vehicleVelocity);
	glm::vec3 chassisOrigin = BulletVectorToGlmVec3(m_chassisBody->getCenterOfMassPosition());
	DebugDraw::addLine(chassisOrigin, vehicleNormalVelocity + chassisOrigin, Color::Blue,
	                   Color::Purple, 0.f);

	bool sliding = false;
	{
		for (int wheel = 0; wheel < getNumWheels(); wheel++)
		{
			btWheelInfo& wheelInfo = m_wheelInfo[wheel];
			class btRigidBody* groundObject =
			    (class btRigidBody*)wheelInfo.m_raycastInfo.m_groundObject;

			btScalar rollingFriction = 0.f;

			if (groundObject)
			{
				if (wheelInfo.m_engineForce != 0.f)
				{
					rollingFriction = wheelInfo.m_engineForce * timeStep;
				}
				else
				{
					btScalar defaultRollingFrictionImpulse = 0.f;
					btScalar maxImpulse =
					    wheelInfo.m_brake ? wheelInfo.m_brake : defaultRollingFrictionImpulse;
					btWheelContactPoint contactPt(m_chassisBody, groundObject,
					                              wheelInfo.m_raycastInfo.m_contactPointWS,
					                              m_forwardWS[wheel], maxImpulse);
					btAssert(numWheelsOnGround > 0);
					rollingFriction = calcRollingFriction(contactPt, numWheelsOnGround);
				}
			}

			// switch between active rolling (throttle), braking and non-active rolling friction
			// (no throttle/break)

			m_forwardImpulse[wheel] = btScalar(0.);
			m_wheelInfo[wheel].m_skidInfo = btScalar(1.);

			if (groundObject)
			{
				m_wheelInfo[wheel].m_skidInfo = btScalar(1.);

				btScalar maximp =
				    wheelInfo.m_wheelsSuspensionForce * timeStep * wheelInfo.m_frictionSlip;
				btScalar maximpSide = maximp;

				btScalar maximpSquared = maximp * maximpSide;

				m_forwardImpulse[wheel] = rollingFriction;  // wheelInfo.m_engineForce*
				                                            // timeStep;

				btScalar x = (m_forwardImpulse[wheel]) * fwdFactor;
				btScalar y = (m_sideImpulse[wheel]) * sideFactor;

				if (hackySlide)
				{
					btScalar impulseAbs =
					    m_sideImpulse[wheel] < 0.f ? -m_sideImpulse[wheel] : m_sideImpulse[wheel];
					if (impulseAbs > 200.f)
					{
						maximpSquared = 0.f;
						maximp = 0.f;
						// if (debugOutput)
							// LOGV << "[" << wheel << "] " << y << " side impulse";
					}
				}

				btScalar impulseSquared = (x * x + y * y);

				glm::mat4 wheelWSTransform = BulletTransformToGlmMat4(getWheelTransformWS(wheel));
				glm::vec3 wheelNormalDirection = RotateGlmVec3ByMat4(wheelWSTransform, ForwardAxis);
				glm::mat4 chassisTransform =
				    BulletTransformToGlmMat4(m_chassisBody->getCenterOfMassTransform());
				// Rotate front wheels by steering angle
				glm::vec3 wheelDirectionLocalSpace = RotateGlmVec3ByMat4(
				    glm::rotate(glm::mat4(1.f), glm::radians(wheelInfo.m_steering), UpAxis),
				    ForwardAxis);
				glm::vec3 wheelDirection =
				    RotateGlmVec3ByMat4(chassisTransform, wheelDirectionLocalSpace);
				// TODO: front tires need their rotation added
				float slipAngle = glm::angle(vehicleNormalVelocity, wheelDirection);
				isSliding = glm::length(vehicleVelocity) > 1.f &&
				            glm::abs(slipAngle) > glm::radians(1.f);
				if (debugOutput)
				{
					LOGV << "[" << wheel << "] " << glm::degrees(glm::abs(slipAngle))
					     << " slip angle steering " << wheelInfo.m_steering;
				}
				if (isSliding)
					m_wheelInfo[wheel].m_skidInfo = 0.2f;

				if (isSliding)
				{
					// glm::vec3 wheelOrigin = BulletVectorToGlmVec3(localTransform.getOrigin());
					const btTransform& wheelTrans = getWheelTransformWS(wheel);
					glm::vec3 wheelPosWorldSpace = BulletVectorToGlmVec3(wheelTrans.getOrigin());
					// TODO Use actual ground offset
					glm::vec3 offsetToGround =
					    RotateGlmVec3ByMat4(chassisTransform, glm::vec3(0.f, -0.6209f, 0.f));
					DebugDraw::addLine(offsetToGround + wheelPosWorldSpace,
					                   (wheelPosWorldSpace + wheelDirection) + offsetToGround,
					                   Color::Red, Color::Red, 10.f);
				}

				if (impulseSquared > maximpSquared)
				{
					sliding = true;

					btScalar factor = maximp / btSqrt(impulseSquared);

					m_wheelInfo[wheel].m_skidInfo *= factor;
				}
			}
		}
	}

	if (sliding)
	{
		for (int wheel = 0; wheel < getNumWheels(); wheel++)
		{
			if (m_sideImpulse[wheel] != btScalar(0.))
			{
				if (m_wheelInfo[wheel].m_skidInfo < btScalar(1.))
				{
					m_forwardImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
					m_sideImpulse[wheel] *= m_wheelInfo[wheel].m_skidInfo;
					if (debugOutput)
						LOGV << "[" << wheel << "] " << m_sideImpulse[wheel] << " side impulse";
				}
			}
		}
	}

	// apply the impulses
	{
		for (int wheel = 0; wheel < getNumWheels(); wheel++)
		{
			btWheelInfo& wheelInfo = m_wheelInfo[wheel];

			btVector3 rel_pos =
			    wheelInfo.m_raycastInfo.m_contactPointWS - m_chassisBody->getCenterOfMassPosition();

			if (m_forwardImpulse[wheel] != btScalar(0.))
			{
				m_chassisBody->applyImpulse(m_forwardWS[wheel] * (m_forwardImpulse[wheel]),
				                            rel_pos);
			}
			if (m_sideImpulse[wheel] != btScalar(0.))
			{
				class btRigidBody* groundObject =
				    (class btRigidBody*)m_wheelInfo[wheel].m_raycastInfo.m_groundObject;

				btVector3 rel_pos2 = wheelInfo.m_raycastInfo.m_contactPointWS -
				                     groundObject->getCenterOfMassPosition();

				btVector3 sideImp = m_axle[wheel] * m_sideImpulse[wheel];

#if defined ROLLING_INFLUENCE_FIX  // fix. It only worked if car's up was along Y - VT.
				btVector3 vChassisWorldUp =
				    getRigidBody()->getCenterOfMassTransform().getBasis().getColumn(m_indexUpAxis);
				rel_pos -= vChassisWorldUp *
				           (vChassisWorldUp.dot(rel_pos) * (1.f - wheelInfo.m_rollInfluence));
#else
				rel_pos[m_indexUpAxis] *= wheelInfo.m_rollInfluence;
#endif
				m_chassisBody->applyImpulse(sideImp, rel_pos);

				// apply friction impulse on the ground
				groundObject->applyImpulse(-sideImp, rel_pos2);
			}
		}
	}
}

PhysicsVehicle::PhysicsVehicle(PhysicsWorld& physicsWorld, PhysicsVehicleTuning* newVehicleTuning)
    : ownerWorld(physicsWorld), vehicleTuning(newVehicleTuning)
{
	PerfTimeNamedScope(vehicleInit, "Vehicle constructor", tracy::Color::Tomato);

	// Draw the positions of wheels and various other things at 0, 0, 0
	bool debugDrawLayout = true;

	float chassisWidthHalfExtents = vehicleTuning->chassisWidth / 2.f;
	float chassisLengthHalfExtents = vehicleTuning->chassisLength / 2.f;

	glm::vec3 chassisOrigin;

	// Create chassis
	{
		btCollisionShape* chassisShape = new btBoxShape(btVector3(
		    chassisWidthHalfExtents, vehicleTuning->chassisHeight / 2, chassisLengthHalfExtents));
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
		localTransform.setOrigin(vehicleTuning->chassisLocalOffset);

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
		transform.setOrigin(btVector3(0.f, 0.f, 0.f));

		carChassis = physicsWorld.localCreateRigidBody(vehicleTuning->massKg, transform, compound);
		// carChassis->setDamping(0.2,0.2);

		/// never deactivate the vehicle
		// TODO: This may be a good optimization in the future to do for stationary vehicles
		carChassis->setActivationState(DISABLE_DEACTIVATION);

		if (debugDrawLayout)
		{
			chassisOrigin = BulletVectorToGlmVec3(transform.getOrigin());
			glm::vec3 localOrigin = BulletVectorToGlmVec3(localTransform.getOrigin());
			DebugDraw::addLine(chassisOrigin, localOrigin + chassisOrigin, Color::Yellow,
			                   Color::Purple, 400.f);
		}
	}

	vehicleRayCaster = new btDefaultVehicleRaycaster(physicsWorld.world);
	vehicle = new CustomRaycastVehicle(suspensionAndFrictionTuning, carChassis, vehicleRayCaster);

	physicsWorld.world->addVehicle(vehicle);

	// Create wheels
	{
		bool isFrontWheel = true;

		// choose coordinate system
		vehicle->setCoordinateSystem(rightAxisIndex, upAxisIndex, forwardAxisIndex);

		btVector3 connectionPointCS0(chassisWidthHalfExtents - (0.3 * vehicleTuning->wheelWidth),
		                             vehicleTuning->connectionHeight, chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, vehicleTuning->wheelDirectionCS0,
		                  vehicleTuning->wheelAxleCS, vehicleTuning->suspensionRestLength,
		                  vehicleTuning->wheelRadius, suspensionAndFrictionTuning, isFrontWheel);

		if (debugDrawLayout)
		{
			glm::vec3 connectionPoint = BulletVectorToGlmVec3(connectionPointCS0);

			// Wheel connection point (top of wheel relative to chassis origin)
			DebugDraw::addLine(chassisOrigin, connectionPoint + chassisOrigin, Color::Green,
			                   Color::Blue, 400.f);
			// Axle (normal)
			DebugDraw::addLine(chassisOrigin,
			                   BulletVectorToGlmVec3(vehicleTuning->wheelAxleCS) + chassisOrigin,
			                   Color::Orange, Color::Red, 400.f);
			// Wheel radius
			DebugDraw::addLine(
			    connectionPoint,
			    connectionPoint + chassisOrigin + glm::vec3(0.f, -vehicleTuning->wheelRadius, 0.f),
			    Color::Green, Color::Red, 400.f);
			// Wheel direction (normal) (normally -UpAxis)
			DebugDraw::addLine(
			    chassisOrigin,
			    chassisOrigin + BulletVectorToGlmVec3(vehicleTuning->wheelDirectionCS0),
			    Color::Green, Color::Red, 400.f);
			// Connection height (offset)
			DebugDraw::addLine(chassisOrigin + glm::vec3(1.f, 0.f, 0.f),
			                   chassisOrigin + glm::vec3(1.f, vehicleTuning->connectionHeight, 0.f),
			                   Color::Blue, Color::Orange, 400.f);

			// Example raycast
			glm::vec3 wheelConnectionWithOffset =
			    connectionPoint + chassisOrigin + glm::vec3(1.f, 0.f, 0.f);
			float rayLength = vehicleTuning->wheelRadius + vehicleTuning->suspensionRestLength;
			glm::vec3 wheelRay = wheelConnectionWithOffset +
			                     glm::vec3(vehicleTuning->wheelDirectionCS0[0] * rayLength,
			                               vehicleTuning->wheelDirectionCS0[1] * rayLength,
			                               vehicleTuning->wheelDirectionCS0[2] * rayLength);
			DebugDraw::addLine(wheelConnectionWithOffset, wheelRay, Color::Purple, Color::Purple,
			                   400.f);
		}

		connectionPointCS0 = btVector3(-chassisWidthHalfExtents + (0.3 * vehicleTuning->wheelWidth),
		                               vehicleTuning->connectionHeight, chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, vehicleTuning->wheelDirectionCS0,
		                  vehicleTuning->wheelAxleCS, vehicleTuning->suspensionRestLength,
		                  vehicleTuning->wheelRadius, suspensionAndFrictionTuning, isFrontWheel);

		if (debugDrawLayout)
		{
			glm::vec3 connectionPoint = BulletVectorToGlmVec3(connectionPointCS0);
			DebugDraw::addLine(chassisOrigin, connectionPoint + chassisOrigin, Color::Green,
			                   Color::Blue, 400.f);
		}

		isFrontWheel = false;
		connectionPointCS0 = btVector3(chassisWidthHalfExtents - (0.3 * vehicleTuning->wheelWidth),
		                               vehicleTuning->connectionHeight, -chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, vehicleTuning->wheelDirectionCS0,
		                  vehicleTuning->wheelAxleCS, vehicleTuning->suspensionRestLength,
		                  vehicleTuning->wheelRadius, suspensionAndFrictionTuning, isFrontWheel);

		if (debugDrawLayout)
		{
			glm::vec3 connectionPoint = BulletVectorToGlmVec3(connectionPointCS0);
			DebugDraw::addLine(chassisOrigin, connectionPoint + chassisOrigin, Color::Green,
			                   Color::Blue, 400.f);
		}

		connectionPointCS0 = btVector3(-chassisWidthHalfExtents + (0.3 * vehicleTuning->wheelWidth),
		                               vehicleTuning->connectionHeight, -chassisLengthHalfExtents);
		vehicle->addWheel(connectionPointCS0, vehicleTuning->wheelDirectionCS0,
		                  vehicleTuning->wheelAxleCS, vehicleTuning->suspensionRestLength,
		                  vehicleTuning->wheelRadius, suspensionAndFrictionTuning, isFrontWheel);

		if (debugDrawLayout)
		{
			glm::vec3 connectionPoint = BulletVectorToGlmVec3(connectionPointCS0);
			DebugDraw::addLine(chassisOrigin, connectionPoint + chassisOrigin, Color::Green,
			                   Color::Blue, 400.f);
		}

		for (int i = 0; i < vehicle->getNumWheels(); i++)
		{
			btWheelInfo& wheel = vehicle->getWheelInfo(i);
			wheel.m_suspensionStiffness = vehicleTuning->suspensionStiffness;
			wheel.m_wheelsDampingRelaxation = vehicleTuning->suspensionDampingRelaxation;
			wheel.m_wheelsDampingCompression = vehicleTuning->suspensionDampingCompression;
			wheel.m_frictionSlip = vehicleTuning->wheelFriction;
			wheel.m_rollInfluence = vehicleTuning->rollInfluence;
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

	{
		const std::lock_guard<std::mutex> lock(g_vehiclesMutex);
		g_vehicles.push_back(this);
	}
}

PhysicsVehicle::~PhysicsVehicle()
{
	const std::lock_guard<std::mutex> lock(g_vehiclesMutex);
	g_vehicles.erase(std::remove(g_vehicles.begin(), g_vehicles.end(), this), g_vehicles.end());

	delete vehicleRayCaster;
	delete vehicle;
}

void PhysicsVehicle::Reset()
{
	VehicleSteering = 0.f;
	BrakingForce = vehicleTuning->defaultBrakingForce;
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

	lastUpdateTransform = GetInterpolatedTransform();

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
	if (selectedGear >= static_cast<int>(gearboxRatios.size()))
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

		// LOGD << "Rear wheel max Delta Rotation = " << maxDeltaRotation;

		// TODO This isn't actually per second yet
		engineRadiansPerStep = maxDeltaRotation * gearboxRatio;
	}

	// TODO Need to take the same step from the wheel update calculations to get the same result
	// TODO Some scary constants in here (like the 60)
	engineRpmOut = (engineRadiansPerStep * (1.f / (deltaTime == 0.f ? 0.001f : deltaTime))) *
	               (1.f / (2.f * glm::pi<float>())) * (60.f / 1.f);

	// TODO This is not good
	// Handle idle (manuals should stall I think? need to do some research on this)
	if (glm::abs(engineRpmOut) < idleEngineRpm)
		engineRpmOut = engineRpmOut < 0.f ? -idleEngineRpm : idleEngineRpm;

	// Blown engine
	if (glm::abs(engineRpmOut) > maxEngineRpm)
		engineRpmOut = engineRpmOut < 0.f ? -maxEngineRpm : maxEngineRpm;

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
	vehicle->debugOutput = debugOutput;

	// Engine force
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
		// LOGV << "Shifted from " << gearBeforeAutoShift << " to " << SelectedGear;
		playVehicleShifting();
	}

	// LOGV << "Input throttle: " << ThrottlePercent << " Gear: " << SelectedGear
	// << " output force: " << engineForce;

	// Apply forces
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

	// Update position. This is what everything outside the physics engine will use from now on
	glm::mat4 chassisTransform = GetInterpolatedTransform();
	lastUpdateTransform = chassisTransform;

	// Update wheel transforms. It's important to do this near GetInterpolatedTransform() so they
	// match each other
	for (int i = 0; i < vehicle->getNumWheels(); i++)
	{
		// Synchronize the wheels with the (interpolated) chassis worldtransform
		vehicle->updateWheelTransform(i, true);

		btTransform tr = vehicle->getWheelInfo(i).m_worldTransform;
		glm::mat4 wheelMatrix = BulletTransformToGlmMat4(tr);
		// Rotate wheels 1 and 3 (right side) so hubcap faces outwards
		if (i % 2 != 0)
		{
			glm::vec3 rotateYAxis = {0.f, 1.f, 0.f};

			glm::mat4 rotateTireY = glm::rotate(glm::mat4(1.f), glm::radians(180.f), rotateYAxis);
			wheelMatrix = wheelMatrix * rotateTireY;
		}

		lastUpdateWheelTransforms[i] = wheelMatrix;
	}
}

glm::mat4 PhysicsVehicle::GetWheelTransform(int wheelIndex) const
{
	return lastUpdateWheelTransforms[wheelIndex];
}

glm::mat4 PhysicsVehicle::GetTransform() const
{
	return lastUpdateTransform;
}

glm::vec3 PhysicsVehicle::GetPosition() const
{
	return lastUpdateTransform[3];
}

glm::mat4 PhysicsVehicle::GetInterpolatedTransform() const
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

glm::vec3 PhysicsVehicle::GetInterpolatedPosition() const
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

void PhysicsVehicle::SetTransform(const glm::mat4& transform) const
{
	carChassis->setCenterOfMassTransform(GlmMat4ToBulletTransform(transform));
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
	const std::lock_guard<std::mutex> lock(g_vehiclesMutex);

	if (!g_vehicles.empty())
	{
		PhysicsVehicle* playerVehicle = g_vehicles[0];
		const std::lock_guard<std::mutex> lock(playerVehicle->engineDetailsMutex);
		return playerVehicle->engineRpm;
	}
	return 0.f;
}
