#include "Joystick.hpp"

// Device Joystick
#include <SFML/Window.hpp>

#include "PhysicsVehicle.hpp"

#include "DebugDisplay.hpp"
#include "Math.hpp"

// For abs
#include "glm/common.hpp"

#include <iostream>
#include <sstream>

const float joystickRange = 100.f;

void printJoystickButtonPresses()
{
	int playerJoystickId = 0;
	bool connected = sf::Joystick::isConnected(playerJoystickId);
	if (!connected)
		return;
	// How many buttons does joystick #0 support?
	unsigned int buttons = sf::Joystick::getButtonCount(playerJoystickId);
	for (int i = 0; i < buttons; ++i)
	{
		bool buttonPressed = sf::Joystick::isButtonPressed(playerJoystickId, i);
		if (buttonPressed)
			std::cout << "Pressed button " << i << "\n";
	}
}

void printJoystickInput()
{
	int playerJoystickId = 0;

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

	std::cout << "connected = " << connected << " buttons = " << buttons << " hasX = " << hasX
	          << " \npositionX = " << positionX << " positionY = " << positionY
	          << " positionZ = " << positionZ << " positionR = " << positionR
	          << " positionU = " << positionU << " positionV = " << positionV << "\n";

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

	// Is joystick #0 connected?
	int playerJoystickId = 0;
	bool controllerConnected = sf::Joystick::isConnected(playerJoystickId);
	// TODO Support controller select (later in project)
	if (!controllerConnected)
		return;

	// Start button resets vehicle
	bool startPressed = sf::Joystick::isButtonPressed(playerJoystickId, 7);
	if (startPressed)
		vehicle.Reset();

	// Change tuning via shoulder buttons
	if (sf::Joystick::isButtonPressed(playerJoystickId, 5))
	{
		vehicle.maxEngineForce += 100.f;
		std::cout << "Set maxEngineForce to " << vehicle.maxEngineForce << "\n";
	}
	else if (sf::Joystick::isButtonPressed(playerJoystickId, 4))
	{
		vehicle.maxEngineForce -= 100.f;
		std::cout << "Set maxEngineForce to " << vehicle.maxEngineForce << "\n";
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
				std::cout << "now throttle = " << vehicle.EngineForce
				          << " brake = " << vehicle.BrakingForce << "\n";
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
			std::cout << "now throttle = " << vehicle.EngineForce
			          << " brake = " << vehicle.BrakingForce << "\n";
	}
}
