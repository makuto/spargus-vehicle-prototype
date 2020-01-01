#pragma once

#include <glm/vec3.hpp>  // vec3

class window;
class inputManager;
namespace sf
{
class RenderWindow;
}

struct Camera
{
private:
	float camPos[3] = {0.f, 10.f, 0.f};
	float camRot[3] = {0.f, 0.f, 0.f};
	float camTranslate[3] = {0.f, 0.f, 0.f};
	window& win;
	float add = 1.f;
	float prevY = 0.f;
	float prevX = 0.f;
	sf::RenderWindow* winBase = nullptr;

public:
	Camera(window& winOwner);

	void FreeCam(inputManager& input, float frameTime);

	const float MaxRotateSpeedXDegrees = 90.f;
	glm::vec3 targetCameraDirection = {0.f, 0.f, -1.f};
	void ChaseCamera(double* openGlTargetMatrix);
	// Like chase, but don't constrain the rotation
	void OrbitCamera(double* openGlTargetMatrix);
	
	void UpdateStart();
	void UpdateEnd();
};
