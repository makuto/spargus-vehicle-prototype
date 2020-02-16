#pragma once

#include <vector>

class PhysicsVehicle;
class PhysicsWorld;

namespace GameVehicles
{
PhysicsVehicle* CreateVehicle(PhysicsWorld& world);

void UpdatePhysics(float deltaTime);
void UpdateRender(float deltaTime);
void UpdateAudio(float deltaTime);
}  // namespace GameVehicles
