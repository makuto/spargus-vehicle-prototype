#include "GameVehicle.hpp"

#include "Audio.hpp"
#include "GraphicsObject.hpp"
#include "Math.hpp"
#include "Performance.hpp"
#include "PhysicsVehicle.hpp"
#include "PhysicsWorld.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  //radians
#include <glm/vec3.hpp>           // vec3

namespace GameVehicles
{
struct VehicleGraphics
{
	Graphics::Object chassisRender;
	std::vector<Graphics::Object> wheelRender;
	Graphics::Object basicDriver;
};

struct GameVehicleData
{
	// Physics
	std::vector<PhysicsVehicle*> physicsVehicles;

	// Rendering
	std::vector<VehicleGraphics> vehicleGraphics;

	// Audio
	std::vector<VehicleEngineAudioStream*> vehicleAudio;

	~GameVehicleData()
	{
		for (PhysicsVehicle* vehicle : physicsVehicles)
			delete vehicle;

		// OpenAL will complain if these aren't destroyed
		for (VehicleEngineAudioStream* stream : vehicleAudio)
			delete stream;
	}
};

GameVehicleData g_gameVehicles;

PhysicsVehicle* CreateVehicle(PhysicsWorld& world)
{
	PerfTimeNamedScope(physicsVehicleScope, "Vehicle creation", tracy::Color::OrangeRed);

	PhysicsVehicle* newVehicle = new PhysicsVehicle(world);
	g_gameVehicles.physicsVehicles.push_back(newVehicle);

	// Create associated structures
	g_gameVehicles.vehicleGraphics.emplace_back();
	{
		VehicleGraphics& newGraphics = g_gameVehicles.vehicleGraphics.back();
		newGraphics.chassisRender.Initialize("BasicBuggy_Chassis");
		newGraphics.wheelRender.resize(newVehicle->vehicle->getNumWheels());
		for (int i = 0; i < newVehicle->vehicle->getNumWheels(); ++i)
		{
			Graphics::Object& wheelNode = newGraphics.wheelRender[i];
			// Disable front wheel for now, because the model isn't ready
			wheelNode.Initialize(i < 2 && false ? "Wheel_Front" : "Wheel_Rear");
		}

		newGraphics.basicDriver.Initialize("BasicDriver");
	}

	g_gameVehicles.vehicleAudio.push_back(new VehicleEngineAudioStream());
	g_gameVehicles.vehicleAudio.back()->initializeEngineAudio();

	return newVehicle;
}

void UpdatePhysics(float deltaTime)
{
	PerfTimeNamedScope(physicsVehicleScope, "Vehicle physics", tracy::Color::OrangeRed);

	for (PhysicsVehicle* currentVehicle : g_gameVehicles.physicsVehicles)
	{
		// Make sure vehicle isn't falling through the world
		if (currentVehicle->GetPosition()[1] < -20.f)
		{
			// TODO: Make this put you back in the last known good position?
			currentVehicle->Reset();
		}
		currentVehicle->Update(deltaTime);
	}
}

void UpdateRender(float deltaTime)
{
	PerfTimeNamedScope(graphicsVehicleScope, "Vehicle graphics", tracy::Color::OrangeRed);

	for (size_t i = 0; i < g_gameVehicles.vehicleGraphics.size(); ++i)
	{
		VehicleGraphics& currentGraphics = g_gameVehicles.vehicleGraphics[i];
		PhysicsVehicle* currentVehicle = g_gameVehicles.physicsVehicles[i];

		glm::mat4 chassisTransform = currentVehicle->GetTransform();

		// Chassis rendering
		currentGraphics.chassisRender.SetTransform(chassisTransform);
		currentGraphics.basicDriver.SetTransform(chassisTransform);

		// Wheel rendering
		for (int i = 0; i < currentVehicle->vehicle->getNumWheels(); i++)
		{
			glm::mat4 wheelMatrix = currentVehicle->GetWheelTransform(i);
			// Rotate wheels 1 and 3 (right side) so hubcap faces outwards
			if (i % 2 != 0)
			{
				glm::vec3 rotateYAxis = {0.f, 1.f, 0.f};

				glm::mat4 rotateTireY =
				    glm::rotate(glm::mat4(1.f), glm::radians(180.f), rotateYAxis);
				wheelMatrix = wheelMatrix * rotateTireY;
			}

			currentGraphics.wheelRender[i].SetTransform(wheelMatrix);
		}
	}
}

void UpdateAudio(float deltaTime)
{
	PerfTimeNamedScope(vehicleAudioScope, "Vehicle audio", tracy::Color::OrangeRed);

	for (size_t i = 0; i < g_gameVehicles.vehicleAudio.size(); ++i)
	{
		VehicleEngineAudioStream* currentEngineStream = g_gameVehicles.vehicleAudio[i];
		PhysicsVehicle* currentVehicle = g_gameVehicles.physicsVehicles[i];
		glm::vec3 vehiclePosition = currentVehicle->GetPosition();

		if (currentEngineStream->getStatus() != sf::SoundSource::Status::Playing)
			currentEngineStream->play();

		currentEngineStream->setPosition(vehiclePosition[0], vehiclePosition[1],
		                                 vehiclePosition[2]);
		// TODO This sucks. Make engine noise quieter for the player
		currentEngineStream->setVolume(i == 0 ? 40.f : 100.f);

		currentEngineStream->setPitch(currentVehicle->engineRpm / 10000.f);

		// Update the stream (on another thread) with the current RPM
		currentEngineStream->setEngineRpm(currentVehicle->engineRpm);
	}
}
}  // namespace GameVehicles
