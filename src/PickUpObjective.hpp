#pragma once

class PhysicsWorld;
class window;
namespace PickUpObjectives
{
void Initialize(PhysicsWorld* physicsWorld);
void Update(float frameTime);
void RenderUI(window& win);
}  // namespace PickUpObjectives
