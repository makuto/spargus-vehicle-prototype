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
#include "ModelUtilities/ModelLoader.hpp"
#include "ModelUtilities/ModelToBullet.hpp"
#include "PhysicsVehicle.hpp"
#include "PhysicsWorld.hpp"

// Window variables
// int WindowWidth = 1200;
// int WindowHeight = 700;
int WindowWidth = 1920;
int WindowHeight = 1080;
#define WIN_BACKGROUND_COLOR 20, 20, 20, 255

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
	glViewport(0, 0, width, height);
}

void processInput(inputManager& input, PhysicsVehicle& vehicle)
{
	bool useGameSteering = true;
	// Reset steering and forces immediately
	if (useGameSteering)
	{
		vehicle.EngineForce = 0.f;
		vehicle.BrakingForce = 0.f;
		vehicle.VehicleSteering = 0.f;
	}

	double steeringIncrement = useGameSteering ? vehicle.steeringClamp : vehicle.steeringIncrement;

	// Steering
	if (input.isPressed(inputCode::Left))
	{
		vehicle.VehicleSteering += steeringIncrement;
		if (vehicle.VehicleSteering > vehicle.steeringClamp)
			vehicle.VehicleSteering = vehicle.steeringClamp;
	}

	if (input.isPressed(inputCode::Right))
	{
		vehicle.VehicleSteering -= steeringIncrement;
		if (vehicle.VehicleSteering < -vehicle.steeringClamp)
			vehicle.VehicleSteering = -vehicle.steeringClamp;
	}

	btScalar speedKmHour = vehicle.vehicle->getCurrentSpeedKmHour();

	// Acceleration/braking
	// For the time being, don't allow anything like left foot braking
	if (input.isPressed(inputCode::Up))
	{
		vehicle.EngineForce = vehicle.maxEngineForce;
		vehicle.BrakingForce = 0.f;
	}
	else if (input.isPressed(inputCode::Down))
	{
		// Start going backwards if the brake is held near zero
		if (speedKmHour < 0.1f)
		{
			vehicle.EngineForce = -vehicle.maxEngineForce;
			vehicle.BrakingForce = 0.f;
		}
		else
		{
			vehicle.EngineForce = 0.f;
			vehicle.BrakingForce = vehicle.maxBrakingForce;
		}
	}
	else if (speedKmHour < 0.1f)
	{
		// Auto apply brakes
		vehicle.EngineForce = 0.f;
		vehicle.BrakingForce = vehicle.maxBrakingForce;
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

	// Ground mesh from GLTF
	{
		// if (!LoadModelFromGltf("assets/World.glb", groundModel))
		std::vector<gltf::Mesh<float>> meshes;
		std::vector<gltf::Material> materials;
		std::vector<gltf::Texture> textures;
		bool debugOutput = false;
		const char* groundModelFilename = "assets/World.glb";
		// const char* groundModelFilename = "assets/Plane.glb";
		// const char* groundModelFilename = "assets/Mountain.glb";
		if (!gltf::LoadGLTF(groundModelFilename, /*scale=*/1.f, &meshes, &materials, &textures,
		                    debugOutput))
			return 1;
		for (const gltf::Mesh<float>& mesh : meshes)
			BulletMeshFromGltfMesh(mesh, physicsWorld);
	}

	// initModel(groundModel);

	// GLCallListIndex groundModelCallList = buildCallListFromModel(groundModel);
	// GLCallListIndex groundModelCallList = buildCallListFromModel2(groundModel);

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

		float fieldOfView = 60.f;
		gluPerspective(fieldOfView, mainWindow.getWidth() / mainWindow.getHeight(), 1, 500);
		glRotatef(0, 0, 1, 0);

		// Floor
		glMatrixMode(GL_MODELVIEW);
		{
			groundCallList = glGenLists(1);
			glNewList(groundCallList, GL_COMPILE);
			glBegin(GL_QUADS);

			// Ground
			float groundExtent = 100.f;
			float groundHalfExtent = groundExtent;
			glNormal3f(0, 1, 0);
			glVertex3f(-groundHalfExtent, -0.f, groundHalfExtent);
			glNormal3f(0, 1, 0);
			glVertex3f(groundHalfExtent, -0.f, groundHalfExtent);
			glNormal3f(0, 1, 0);
			glVertex3f(groundHalfExtent, -0.f, -groundHalfExtent);
			glNormal3f(0, 1, 0);
			glVertex3f(-groundHalfExtent, -0.f, -groundHalfExtent);

			glEnd();
			glEndList();
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Lighting
		// float col[4] = {1, 1, 1, 0.0};
		float col[4] = {34 / 255.f, 34 / 255.f, 34 / 255.f, 0.0};
		glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
		// glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 25);

		// Fog
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, col);
		glFogf(GL_FOG_START, 100.f);
		glFogf(GL_FOG_END, 200.);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Game loop
	//

	// A made up but sane first frame
	float lastFrameTime = 0.016f;
	timer frameTimer;
	frameTimer.start();

	Camera cam(mainWindow);
	// bool useChaseCam = true;
	bool useChaseCam = false;

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		if (!useChaseCam)
			cam.FreeCam(input, lastFrameTime);
		cam.UpdateStart();

		processInput(input, vehicle);

		vehicle.Update(lastFrameTime);
		physicsWorld.Update(lastFrameTime);

		// Use vehicle transform to position camera
		if (useChaseCam)
		{
			const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
			btTransform camTransform = vehicleTransform.inverse();
			btScalar vehicleMat[16];
			// vehicleTransform.getOpenGLMatrix(vehicleMat);
			camTransform.getOpenGLMatrix(vehicleMat);

			cam.ChaseCamera(vehicleMat);
		}

		// glCallList(groundCallList);

		physicsWorld.DebugRender();

		cam.UpdateEnd();

		// Finished physics update and drawing; send it on its way
		mainWindow.update();

		lastFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	return 0;
}
