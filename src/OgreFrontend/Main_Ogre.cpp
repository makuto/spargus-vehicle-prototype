#include <iostream>

#include "Mogre.hpp"

#include "Game.hpp"
#include "Math.hpp"
#include "PhysicsVehicle.hpp"

Mogre::ThreadData* gThreadData = nullptr;

struct RenderVehicle
{
	Mogre::GameEntity* entity = nullptr;

	float position[3] = {0.f, 0.f, 0.f};
	float rotation[4] = {0.f, 0.f, 0.f, -1.f};
	float scale[3] = {1.f, 1.f, 1.f};
};

RenderVehicle gRenderVehicle;

void Initialize(Mogre::ThreadData* threadData)
{
	InitializeGame();

	PositionRotationFromBulletTransform(gPhysicsState->vehicle.vehicle->getChassisWorldTransform(),
	                                    gRenderVehicle.position, gRenderVehicle.rotation);
	gRenderVehicle.entity =
	    Mogre::CreateGameEntity(gThreadData, "Cube_d.mesh", gRenderVehicle.position,
	                            gRenderVehicle.rotation, gRenderVehicle.scale);

	// Mogre::CreateGameEntity(gThreadData, "World.glb", gRenderVehicle.position,
	//                         gRenderVehicle.rotation, gRenderVehicle.scale);
}

// Not render update
void Update(Mogre::ThreadData* threadData, float frameTime)
{
	UpdateGame(frameTime);

	// std::cout << frameTime << " " << entity << "\n";
	// gRenderVehicle.position[1] += 10.f * frameTime;
	PositionRotationFromBulletTransform(gPhysicsState->vehicle.vehicle->getChassisWorldTransform(),
	                                    gRenderVehicle.position, gRenderVehicle.rotation);
	MoveGameEntity(gThreadData, gRenderVehicle.entity, gRenderVehicle.position,
	               gRenderVehicle.rotation);
}

int main()
{
	Mogre::SetInitializeCallback(Initialize);
	Mogre::SetUpdateCallback(Update);

	gThreadData = Mogre::Initialize();

	Mogre::Run(gThreadData);

	return 0;
}
