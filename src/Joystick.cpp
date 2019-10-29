#include "Joystick.hpp"

// Device Joystick
#include <SFML/Window.hpp>

#include "PhysicsVehicle.hpp"

// For abs
#include "glm/common.hpp"

#include <iostream>

const float joystickRange = 100.f;

void printJoystickInput()
{
	// Is joystick #0 connected?
	bool connected = sf::Joystick::isConnected(0);
	// How many buttons does joystick #0 support?
	unsigned int buttons = sf::Joystick::getButtonCount(0);
	// Does joystick #0 define a X axis?
	bool hasX = sf::Joystick::hasAxis(0, sf::Joystick::X);
	// Is button #2 pressed on joystick #0? (XB)
	bool pressed = sf::Joystick::isButtonPressed(0, 2);

	// What's the current position of the X axis on joystick #0? (Left thumbstick; -100 = left)
	float positionX = sf::Joystick::getAxisPosition(0, sf::Joystick::X);
	// What's the current position of the Y axis on joystick #0? (Left thumbstick; -100 = forward)
	float positionY = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
	// Left trigger pressed = 100 (unpressed = -100)
	float positionZ = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);
	// Right trigger pressed = 100 (unpressed = -100)
	float positionR = sf::Joystick::getAxisPosition(0, sf::Joystick::R);
	// Right joystick X left -100
	float positionU = sf::Joystick::getAxisPosition(0, sf::Joystick::U);
	// Right joystick Y forward -100
	float positionV = sf::Joystick::getAxisPosition(0, sf::Joystick::V);

	std::cout << "connected = " << connected << " buttons = " << buttons << " hasX = " << hasX
	          << " pressed = " << pressed << " \npositionX = " << positionX
	          << " positionY = " << positionY << " positionZ = " << positionZ
	          << " positionR = " << positionR << " positionU = " << positionU
	          << " positionV = " << positionV << "\n";
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
	// printJoystickInput();
	
	bool useGameSteering = true;
	// Reset steering and forces immediately
	if (useGameSteering)
	{
		vehicle.EngineForce = 0.f;
		vehicle.BrakingForce = 0.f;
		vehicle.VehicleSteering = 0.f;
	}

	// Is joystick #0 connected?
	bool connected = sf::Joystick::isConnected(0);
	// TODO Support controller select (later in project)
	if (!connected)
		return;

	// X button
	// bool pressed = sf::Joystick::isButtonPressed(0, 2);
	
	// -100 = left thumbstick up, 100 = down
	// float position = sf::Joystick::getAxisPosition(0, sf::Joystick::Y);
	
	// -100 = left thumbstick up, 100 = down
	float turningPosition = sf::Joystick::getAxisPosition(0, sf::Joystick::X) * -1.f;
	
	// Right trigger pressed = 100 (unpressed = -100)
	float throttlePosition = sf::Joystick::getAxisPosition(0, sf::Joystick::R);
	
	// Left trigger pressed = 100 (unpressed = -100)
	float brakePosition = sf::Joystick::getAxisPosition(0, sf::Joystick::Z);

	applyDeadzone(turningPosition);
	applyDeadzone(throttlePosition);
	applyDeadzone(brakePosition);

	vehicle.VehicleSteering = interpolateRange(-vehicle.steeringClamp, vehicle.steeringClamp,
	                                           -joystickRange, joystickRange, turningPosition);
	vehicle.EngineForce = interpolateRange(0.f, vehicle.maxEngineForce,
	                                       -joystickRange, joystickRange, throttlePosition);
	vehicle.BrakingForce = interpolateRange(0.f, vehicle.maxBrakingForce,
	                                        -joystickRange, joystickRange, brakePosition);

	btScalar speedKmHour = vehicle.vehicle->getCurrentSpeedKmHour();
	std::cout << "speedKmHour = " << speedKmHour << " throttle = " << vehicle.EngineForce
	          << " brake = " << vehicle.BrakingForce << "\n";

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
		std::cout << "now throttle = " << vehicle.EngineForce << " brake = " << vehicle.BrakingForce
		          << "\n";
	}
}
