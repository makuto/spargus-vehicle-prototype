#include <iostream>

#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <GL/glew.h>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <map>

#include "Camera.hpp"
#include "Render_Horde3D.hpp"
#include <Horde3D.h>
#include <Horde3DUtils.h>

// Window variables
// int WindowWidth = 1200;
// int WindowHeight = 700;
int WindowWidth = 1920;
int WindowHeight = 1080;
#define WIN_BACKGROUND_COLOR 20, 20, 20, 255

void initializeWindow(window& win)
{
	{
		sf::ContextSettings settings = win.getBase()->getSettings();

		std::cout << "depth bits:" << settings.depthBits << std::endl;
		std::cout << "stencil bits:" << settings.stencilBits << std::endl;
		std::cout << "antialiasing level:" << settings.antialiasingLevel << std::endl;
		std::cout << "version:" << settings.majorVersion << "." << settings.minorVersion
		          << std::endl;
	}
	win.setBackgroundColor(WIN_BACKGROUND_COLOR);

	// shouldClose manages some state
	win.shouldClose();
	win.update();

	win.shouldClear(true);
	win.getBase()->setVerticalSyncEnabled(true);
	win.getBase()->setFramerateLimit(60);
}

void windowResizeCB(float width, float height)
{
	WindowWidth = width;
	WindowHeight = height;

	// Is this necessary?
	glViewport(0, 0, width, height);
}

// See https://www.sfml-dev.org/tutorials/2.5/window-opengl.php
struct WindowScopedContextActivate
{
	window& win;
	WindowScopedContextActivate(window& newWin) : win(newWin)
	{
		win.getBase()->setActive(true);
	}

	~WindowScopedContextActivate()
	{
		win.getBase()->setActive(false);
	}
};

int main()
{
	std::cout << "Spargus Vehicle Prototype\n";

	////////////////////////////////////////////////////////////////////////////////
	// Initialization
	//

	window mainWindow(WindowWidth, WindowHeight, "Spargus Vehicle Prototype", &windowResizeCB);
	initializeWindow(mainWindow);

	{
		WindowScopedContextActivate activate(mainWindow);
		hordeInitialize(WindowWidth, WindowHeight);
	}

	inputManager input(&mainWindow);

	// initModel(groundModel);

	// GLCallListIndex groundModelCallList = buildCallListFromModel(groundModel);
	// GLCallListIndex groundModelCallList = buildCallListFromModel2(groundModel);

	////////////////////////////////////////////////////////////////////////////////
	// Game loop
	//

	// A made up but sane first frame
	float previousFrameTime = 0.016f;
	timer frameTimer;
	frameTimer.start();

	// Camera cam(mainWindow);
	// bool useChaseCam = true;
	// // bool useChaseCam = false;

	mainWindow.shouldClear(false);

	float cameraPos[] = {0.f, 0.f, 100.f};

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		mainWindow.getBase()->setActive(true);

		// if (!useChaseCam)
		// 	cam.FreeCam(input);
		// cam.UpdateStart();

		// // Use vehicle transform to position camera
		// if (useChaseCam)
		// {
		// 	const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
		// 	btTransform camTransform = vehicleTransform.inverse();
		// 	btScalar vehicleMat[16];
		// 	// vehicleTransform.getOpenGLMatrix(vehicleMat);
		// 	camTransform.getOpenGLMatrix(vehicleMat);
		// 	// h3dSetNodeTransform(hordeCam, 0, 20, 0, /*rotationEuler=*/0, 0, 0, /*scaling=*/1, 1,
		// 	// 1);

		// 	cam.ChaseCamera(vehicleMat);
		// }

		{
			float camSpeed = 20000.f;
			if (input.isPressed(inputCode::Left))
			{
				cameraPos[0] -= camSpeed * frameTimer.getTime();
			}

			if (input.isPressed(inputCode::Right))
			{
				cameraPos[0] += camSpeed * frameTimer.getTime();
			}

			if (input.isPressed(inputCode::Up))
			{
				cameraPos[2] -= camSpeed * frameTimer.getTime();				
			}
			else if (input.isPressed(inputCode::Down))
			{
				cameraPos[2] += camSpeed * frameTimer.getTime();				
			}

			// std::cout << cameraPos[0] << ", " << cameraPos[1] << ", " << cameraPos[2] << "\n";
			h3dSetNodeTransform(hordeCamera, cameraPos[0], cameraPos[1], cameraPos[2], 0, 0, 0, 1,
			                    1, 1);
		}

		// std::cout << previousFrameTime << "\n";
		hordeUpdate(previousFrameTime);
		GLenum eError = glGetError();
		if (eError)
			std::cout << eError;

		// cam.UpdateEnd();

		// Finished physics update and drawing; send it on its way
		mainWindow.update();
		eError = glGetError();
		if (eError)
			std::cout << eError;

		mainWindow.getBase()->setActive(false);

		previousFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	hordeRelease();

	return 0;
}
