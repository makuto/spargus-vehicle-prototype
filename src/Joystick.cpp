#include "Joystick.hpp"

// Device Joystick
#include <SFML/Window.hpp>

#include "Camera.hpp"
#include "PhysicsVehicle.hpp"

#include "DebugDisplay.hpp"
#include "Logging.hpp"
#include "Math.hpp"

// For abs
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  //radians
#include <glm/vec3.hpp>           // vec3
#include "glm/common.hpp"

#include <iostream>
#include <sstream>

const float joystickRange = 100.f;

int getPlayerJoystickId()
{
	int playerJoystickId = 0;
	if (!sf::Joystick::isConnected(playerJoystickId))
		return -1;
	else
		return playerJoystickId;
}

void printJoystickButtonPresses()
{
	int playerJoystickId = getPlayerJoystickId();
	if (playerJoystickId < 0)
		return;
	// How many buttons does joystick #0 support?
	unsigned int buttons = sf::Joystick::getButtonCount(playerJoystickId);
	for (unsigned int i = 0; i < buttons; ++i)
	{
		bool buttonPressed = sf::Joystick::isButtonPressed(playerJoystickId, i);
		if (buttonPressed)
			LOGD << "Pressed button " << i;
	}
}

void printJoystickInput()
{
	int playerJoystickId = getPlayerJoystickId();
	if (playerJoystickId < 0)
		return;

	// Is joystick #0 connected?
	bool connected = sf::Joystick::isConnected(playerJoystickId);
	// How many buttons does joystick #0 support?
	unsigned int buttons = sf::Joystick::getButtonCount(playerJoystickId);
	// Does joystick #0 define a X axis?
	bool hasX = sf::Joystick::hasAxis(playerJoystickId, sf::Joystick::X);

	// What's the current position of the X axis on joystick #0? (Left thumbstick; -100 = left)
	float positionX = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::X);
	// What's the current position of the Y axis on joystick #0? (Left thumbstick; -100 = forward)
	float positionY = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::Y);
	// Left trigger pressed = 100 (unpressed = -100)
	float positionZ = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::Z);
	// Right trigger pressed = 100 (unpressed = -100)
	float positionR = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::R);
	// Right joystick X left -100
	float positionU = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::U);
	// Right joystick Y forward -100
	float positionV = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::V);

	LOGD << "connected = " << connected << " buttons = " << buttons << " hasX = " << hasX
	     << " \npositionX = " << positionX << " positionY = " << positionY
	     << " positionZ = " << positionZ << " positionR = " << positionR
	     << " positionU = " << positionU << " positionV = " << positionV;

	printJoystickButtonPresses();
}

void applyDeadzone(float& joystickValue)
{
	const float deadzone = 5.f;
	if (glm::abs(joystickValue) < deadzone)
		joystickValue = 0.f;
}

float interpolateRange(float startA, float endA, float startB, float endB, float bValue)
{
	float interpolateTo = (bValue - startB) / (endB - startB);
	return (interpolateTo * (endA - startA)) + startA;
}

void processVehicleInputJoystick(PhysicsVehicle& vehicle)
{
	bool debugPrint = false;

	// printJoystickButtonPresses();

	int playerJoystickId = getPlayerJoystickId();
	if (playerJoystickId < 0)
		return;

	// Start button resets vehicle
	bool startPressed = sf::Joystick::isButtonPressed(playerJoystickId, 7);
	if (startPressed)
		vehicle.Reset();

	// Change tuning via shoulder buttons
	if (sf::Joystick::isButtonPressed(playerJoystickId, 5))
	{
		vehicle.maxEngineForce += 100.f;
		LOGD << "Set maxEngineForce to " << vehicle.maxEngineForce;
	}
	else if (sf::Joystick::isButtonPressed(playerJoystickId, 4))
	{
		vehicle.maxEngineForce -= 100.f;
		LOGD << "Set maxEngineForce to " << vehicle.maxEngineForce;
	}

	if (vehicle.maxEngineForce < 0.f)
		vehicle.maxEngineForce = 0.f;

	// -100 = left thumbstick up, 100 = down
	// float position = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);

	// -100 = left thumbstick up, 100 = down
	float turningPosition = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::X) * -1.f;

	// Right trigger pressed = 100 (unpressed = -100)
	float throttlePosition = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::R);

	// Left trigger pressed = 100 (unpressed = -100)
	float brakePosition = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::Z);

	applyDeadzone(turningPosition);
	applyDeadzone(throttlePosition);
	applyDeadzone(brakePosition);

	bool useGameSteering = true;
	// Reset steering and forces immediately
	if (useGameSteering)
	{
		vehicle.EngineForce = 0.f;
		vehicle.BrakingForce = 0.f;
		vehicle.VehicleSteering = 0.f;
	}

	vehicle.VehicleSteering = interpolateRange(-vehicle.steeringClamp, vehicle.steeringClamp,
	                                           -joystickRange, joystickRange, turningPosition);
	vehicle.EngineForce = interpolateRange(0.f, vehicle.maxEngineForce, -joystickRange,
	                                       joystickRange, throttlePosition);
	vehicle.BrakingForce = interpolateRange(0.f, vehicle.maxBrakingForce, -joystickRange,
	                                        joystickRange, brakePosition);

	btScalar speedKmHour = vehicle.vehicle->getCurrentSpeedKmHour();

	// Auto reverse on brake, if going slow enough (and throttle isn't pressed)
	const float autoReverseThresholdKmHour = 1.f;
	const float autoReverseForceTreshold = 1.f;
	if (vehicle.EngineForce <= 0.f && vehicle.BrakingForce > autoReverseForceTreshold)
	{
		// Start going backwards if the brake is held near zero
		if (speedKmHour < autoReverseThresholdKmHour)
		{
			// Convert braking input into engine input
			vehicle.EngineForce = interpolateRange(0.f, -vehicle.maxEngineForce, 0.f,
			                                       vehicle.maxBrakingForce, vehicle.BrakingForce);
			vehicle.BrakingForce = 0.f;

			if (debugPrint)
				LOGD << "now throttle = " << vehicle.EngineForce
				     << " brake = " << vehicle.BrakingForce;
		}
	}

	// After this speed, apply the brakes when trying to accelerate
	// TODO: Figure out what Jak does for this
	const float autoReverseAccelerateBrakingThresholdKmHour = -30.f;
	if (vehicle.EngineForce > autoReverseForceTreshold && vehicle.BrakingForce == 0.f)
	{
		if (speedKmHour < autoReverseAccelerateBrakingThresholdKmHour)
			vehicle.BrakingForce = vehicle.maxBrakingForce;

		if (debugPrint)
			LOGD << "now throttle = " << vehicle.EngineForce << " brake = " << vehicle.BrakingForce;
	}

	// Air control
	// TODO: Only allow air control if chassis is not contacting anything
	if (!vehicle.WheelsContactingSurface())
	{
		float rollPosition = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::X);
		float pitchPosition =
		    sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::Y) * -1.f;
		applyDeadzone(rollPosition);
		applyDeadzone(pitchPosition);
		glm::vec3 airControlTorque(0.f);
		airControlTorque[0] =
		    interpolateRange(-vehicle.airControlMaxPitchTorque, vehicle.airControlMaxPitchTorque,
		                     -joystickRange, joystickRange, pitchPosition);
		airControlTorque[2] =
		    interpolateRange(-vehicle.airControlMaxRollTorque, vehicle.airControlMaxRollTorque,
		                     -joystickRange, joystickRange, rollPosition);
		// Make torque local to the vehicle
		// TODO: Should this use the interpolated position?
		glm::mat4 vehicleTransform = vehicle.GetTransform();
		for (int i = 0; i < 3; ++i)
			vehicleTransform[3][i] = 0.f;
		glm::vec4 airControlRotate(airControlTorque[0], airControlTorque[1], airControlTorque[2],
		                           1.f);
		airControlRotate = vehicleTransform * airControlRotate;
		glm::vec3 finalAirControl(airControlRotate);
		vehicle.ApplyTorque(finalAirControl);
	}
}

void handleCameraInput(Camera& camera, float frameTime)
{
	int playerJoystickId = getPlayerJoystickId();
	if (playerJoystickId < 0)
		return;

	// Right joystick X left -100
	float cameraJoystickX = sf::Joystick::getAxisPosition(playerJoystickId, sf::Joystick::U);
	applyDeadzone(cameraJoystickX);
	float deltaRotation =
	    interpolateRange(-camera.MaxRotateSpeedXDegrees, camera.MaxRotateSpeedXDegrees,
	                     -joystickRange, joystickRange, cameraJoystickX);
	deltaRotation *= frameTime;
	glm::mat4 camRotateMat = glm::rotate(glm::mat4(1.f), glm::radians(deltaRotation), UpAxis);
	glm::vec4 targetDirection(camera.targetCameraDirection, 1.f);
	targetDirection = targetDirection * camRotateMat;
	for (int i = 0; i < 3; ++i)
		camera.targetCameraDirection[i] = targetDirection[i];
	
	// LOGV << "Delta rotation: " << deltaRotation;
	// LOGV << "Rotation matrix: " << camRotateMat;
	// LOGV << "Target direction: " << camera.targetCameraDirection;
}
