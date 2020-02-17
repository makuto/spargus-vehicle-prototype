#pragma once

#include <glm/mat4x4.hpp>  // mat4

class PhysicsVehicle;
class PhysicsWorld;

namespace GameVehicles
{
PhysicsVehicle* CreateVehicle(PhysicsWorld& world, const glm::mat4& startTransform);

void UpdatePhysics(float deltaTime);
void UpdateRender(float deltaTime);
void UpdateAudio(float deltaTime);
}  // namespace GameVehicles
