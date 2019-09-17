#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

class window;
class inputManager;

struct Camera
{
private:
	float camPos[3] = {0, -10, 0};
	float camRot[3] = {0, 0, 0};
	float camTranslate[3] = {0, 0, -10};
	window& win;
	float add = 1;
	float prevY = 0;
	float prevX = 0;
	sf::RenderWindow* winBase = nullptr;

public:
	Camera(window& winOwner);

	void FreeCam(inputManager& input);
	void ChaseCamera(double* openGlMatrix);

	void UpdateStart();
	void UpdateEnd();
};
