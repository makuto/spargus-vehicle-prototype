#pragma once

#include <glm/vec3.hpp>  // vec3
#include <glm/mat4x4.hpp>  // mat4

class window;
class inputManager;
namespace sf
{
class RenderWindow;
}

struct Camera
{
private:
	glm::vec3 camPos = {0.f, 7.5f, 0.f};
	glm::vec3 camRot = {0.f, 0.f, 0.f};
	glm::vec3 camTranslate = {0.f, 0.f, 0.f};
	window& win;
	float prevY = 0.f;
	float prevX = 0.f;
	sf::RenderWindow* winBase = nullptr;

	// Broadly determines the view angle
	const double cameraHeight = 1.5;
	// Offsets the target down onscreen
	const double targetAddHeight = 1.5;
	const float pullbackRadius = 6.75f;
public:
	Camera(window& winOwner);

	void FreeCam(inputManager& input, float frameTime);

	const float MaxRotateSpeedXDegrees = 90.f;
	glm::vec3 targetCameraDirection = {0.f, 0.f, -1.f};
	void ChaseCamera(const glm::mat4& targetMatrix);
	// Like chase, but don't constrain the rotation
	void OrbitCamera(const glm::mat4& targetMatrix);
	
	void UpdateStart();
	void UpdateEnd();
};
