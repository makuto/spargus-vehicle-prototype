#include <iostream>

#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "PhysicsVehicle.hpp"
#include "PhysicsWorld.hpp"

// Window variables
// int WindowWidth = 1200;
// int WindowHeight = 700;
int WindowWidth = 1920;
int WindowHeight = 1080;
#define WIN_BACKGROUND_COLOR 34, 34, 34, 255

void initializeWindow(window& win)
{
	win.setBackgroundColor(WIN_BACKGROUND_COLOR);

	// shouldClose manages some state
	win.shouldClose();
	win.update();

	win.shouldClear(true);
	win.getBase()->setVerticalSyncEnabled(true);
	win.getBase()->setFramerateLimit(30);
}

void windowResizeCB(float width, float height)
{
	WindowWidth = width;
	WindowHeight = height;

	// Is this necessary?
	glViewport(0,0, width, height);
}

struct Camera
{
	float camPos[3] = {0, -10, 0};
	float camRot[3] = {0, 0, 0};
	float camTranslate[3] = {0, 0, -10};
	window& win;
	float add = 1;
	float prevY = 0;
	float prevX = 0;
	sf::RenderWindow* winBase = nullptr;

	Camera(window& winOwner) : win(winOwner)
	{
		winBase = win.getBase();
		win.getBase()->setMouseCursorVisible(false);
		sf::Mouse::setPosition(sf::Vector2i(win.getWidth() / 2, win.getHeight() / 2), *winBase);
	}

	void UpdateInput(inputManager& input)
	{
		if (input.isPressed(inputCode::W))
		{
			/* yrotrad = (yrot / 180 * 3.141592654f);
			    xrotrad = (xrot / 180 * 3.141592654f);
			    xpos += float(sin(yrotrad)) ;
			    zpos -= float(cos(yrotrad)) ;
			 * */
			camTranslate[0] += float(sin(camRot[1] / 180 * 3.141592654));
			camTranslate[2] -= float(cos(camRot[1] / 180 * 3.141592654));
			// camPos[1]=-10;
		}
		if (input.isPressed(inputCode::S))
		{
			camTranslate[0] -= float(sin(camRot[1] / 180 * 3.141592654));
			camTranslate[2] += float(cos(camRot[1] / 180 * 3.141592654));
			// camTranslate[2]=50*win.GetFrameTime();
			// camPos[1]=-10;
		}
		if (input.isPressed(inputCode::A))
		{
			camTranslate[0] -= float(cos(camRot[1] / 180 * 3.141592654));
			camTranslate[2] -= float(sin(camRot[1] / 180 * 3.141592654));
		}
		if (input.isPressed(inputCode::D))
		{
			camTranslate[0] += float(cos(camRot[1] / 180 * 3.141592654));
			camTranslate[2] += float(sin(camRot[1] / 180 * 3.141592654));
		}

		// camRot[1]=(10*(in->GetMouseX()*win.GetWidth()/10) + 400)/win.GetWidth();
		camRot[1] += (sf::Mouse::getPosition(*winBase).x - prevX) / 5 /**winBase.GetWidth()/3600*/;
		camRot[0] += (sf::Mouse::getPosition(*winBase).y - prevY) / 5;
		prevX = sf::Mouse::getPosition(*winBase).x;
		prevY = sf::Mouse::getPosition(*winBase).y;
		if (prevX <= 100 || prevX >= 200)
		{
			sf::Mouse::setPosition(sf::Vector2i(winBase->getSize().x / 2, winBase->getSize().y / 2), *winBase);
			prevX = winBase->getSize().x / 2;
		}
		if (prevY <= 100 || prevY >= 200)
		{
			sf::Mouse::setPosition(sf::Vector2i(winBase->getSize().x / 2, winBase->getSize().y / 2), *winBase);
			prevY = win.getHeight() / 2;
		}
	}

	void UpdateStart()
	{
		// Set camera
		if (camRot[0] < -90)
			camRot[0] = -90;
		if (camRot[0] > 100)
			camRot[0] = 100;
		// Make the degree positive
		if (camRot[1] < 0)
			camRot[1] += 360;
		// Clamp the degree to 0-360
		if (camRot[1] >= 360)
			camRot[1] -= 360;
		if (camRot[1] <= -360)
			camRot[1] += 360;
		// Calculate the slope of the movement vector
		/*camTranslate[0] *=sin(camRot[1]);
		camTranslate[2] *=cos(camRot[1]);*/
		// Add the translation vector
		camPos[0] += camTranslate[0];
		camPos[1] += camTranslate[1];
		camPos[2] += camTranslate[2];

		// Define the camera matrix
		glLoadIdentity();
		gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
		glRotatef(camRot[0], 1, 0, 0);
		glRotatef(camRot[1], 0, 1, 0);
		glRotatef(camRot[2], 0, 0, 1);

		glTranslatef(-camPos[0], camPos[1], -camPos[2]);
		glPushMatrix();
	}
	
	void UpdateEnd()
	{
		// Reset translation vector
		camTranslate[0] = 0;
		camTranslate[1] = 0;
		camTranslate[2] = 0;

		glPopMatrix();
	}
};

void processInput(inputManager& input, PhysicsVehicle& vehicle)
{
	// Steering
	if (input.isPressed(inputCode::Left))
	{
		vehicle.VehicleSteering += vehicle.steeringIncrement;
		if (vehicle.VehicleSteering > vehicle.steeringClamp)
			vehicle.VehicleSteering = vehicle.steeringClamp;
	}

	if (input.isPressed(inputCode::Right))
	{
		vehicle.VehicleSteering -= vehicle.steeringIncrement;
		if (vehicle.VehicleSteering < -vehicle.steeringClamp)
			vehicle.VehicleSteering = -vehicle.steeringClamp;
	}

	// Acceleration/braking
	if (input.isPressed(inputCode::Up))
	{
		vehicle.EngineForce = vehicle.maxEngineForce;
		vehicle.BreakingForce = 0.f;
	}

	if (input.isPressed(inputCode::Down))
	{
		vehicle.EngineForce = -vehicle.maxEngineForce;
		vehicle.BreakingForce = 0.f;
	}
}

int main()
{
	std::cout << "Spargus Vehicle Prototype\n";

	////////////////////////////////////////////////////////////////////////////////
	// Initialization
	//

	PhysicsWorld physicsWorld;
	PhysicsVehicle vehicle(physicsWorld);

	window mainWindow(WindowWidth, WindowHeight, "Spargus Vehicle Prototype", &windowResizeCB);
	initializeWindow(mainWindow);

	inputManager input(&mainWindow);

	// OpenGL world setup
	int groundCallList = -1;
	{
		glEnable(GL_DEPTH_TEST);
		// glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		float pos[4] = {5, 0, -1, 1};
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1f);
		// glEnable(GL_CULL_FACE);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		gluPerspective(60, mainWindow.getWidth() / mainWindow.getHeight(), 1, 500);
		glRotatef(0, 0, 1, 0);

		// Floor
		glMatrixMode(GL_MODELVIEW);
		{
			groundCallList = glGenLists(1);
			glNewList(groundCallList, GL_COMPILE);
			glBegin(GL_QUADS);

			// Ground
			glNormal3f(0, 1, 0);
			glVertex3f(-50, -5, 0);
			glNormal3f(0, 1, 0);
			glVertex3f(50, -5, 0);
			glNormal3f(0, 1, 0);
			glVertex3f(50, -5, -400);
			glNormal3f(0, 1, 0);
			glVertex3f(-50, -5, -400);

			glEnd();
			glEndList();
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Lighting
		float col[4] = {1, 1, 1, 0.0};
		glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
		// glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 25);
		
		// Fog
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, col);
		glFogf(GL_FOG_START, 200);
		glFogf(GL_FOG_END, 300);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Game loop
	//

	// A made up but sane first frame
	float lastFrameTime = 0.032f;
	timer frameTimer;
	frameTimer.start();

	Camera cam(mainWindow);

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		cam.UpdateInput(input);
		cam.UpdateStart();

		processInput(input, vehicle);

		vehicle.Update(lastFrameTime);
		physicsWorld.Update(lastFrameTime);

		glCallList(groundCallList);

		physicsWorld.DebugRender();

		cam.UpdateEnd();

		// TODO: Next: Lock camera to transform
		const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();

		// Finished physics update and drawing; send it on its way
		mainWindow.update();

		lastFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	return 0;
}
