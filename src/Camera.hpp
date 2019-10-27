#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

class window;
class inputManager;

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
	void ChaseCamera(double* openGlMatrix);

	void UpdateStart();
	void UpdateEnd();
};
