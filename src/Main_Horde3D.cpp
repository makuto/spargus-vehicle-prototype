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

#include <Horde3D.h>
#include "Render_Horde3D.hpp"

#include "linalg.h"

void hordeMatrixFromBulletTransform(const btTransform& transform, float* hordeMatrixOut)
{
	btScalar bulletMat[16];
	transform.getOpenGLMatrix(bulletMat);
	for (size_t i = 0; i < sizeof(bulletMat) / sizeof(bulletMat[0]); i++)
	{
		hordeMatrixOut[i] = bulletMat[i];
	}
}

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

// Copy pasted :(
bool macoyGluInvertMatrix(const float m[16], float invOut[16])
{
	float inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
	         m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
	         m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
	         m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
	          m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
	         m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
	         m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
	         m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
	          m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
	         m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
	         m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
	          m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
	          m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
	         m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
	         m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
	          m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
	          m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}

int main()
{
	std::cout << "Spargus Vehicle Prototype\n";

	////////////////////////////////////////////////////////////////////////////////
	// Initialization
	//

	PhysicsWorld physicsWorld;
	PhysicsVehicle vehicle(physicsWorld);

	// Test bullet serialization
	{
		serializeConcaveMesh();

		btCollisionShape* concaveTestShape = importConcaveMesh();
		if (concaveTestShape)
		{
			btTransform startTransform;
			startTransform.setIdentity();
			startTransform.setOrigin(btVector3(0, -1, 0));
			physicsWorld.localCreateRigidBody(PhysicsWorld::StaticRigidBodyMass, startTransform,
			                                  concaveTestShape);
		}
	}

	window mainWindow(WindowWidth, WindowHeight, "Spargus Vehicle Prototype", &windowResizeCB);
	initializeWindow(mainWindow);

	{
		WindowScopedContextActivate activate(mainWindow);
		hordeInitialize(WindowWidth, WindowHeight);
	}

	inputManager input(&mainWindow);

	// Ground mesh from GLTF
	if (false)
	{
		// if (!LoadModelFromGltf("assets/World.glb", groundModel))
		std::vector<gltf::Mesh<float>> meshes;
		std::vector<gltf::Material> materials;
		std::vector<gltf::Texture> textures;
		bool debugOutput = false;
		// const char* groundModelFilename = "assets/World.glb";
		const char* groundModelFilename = "assets/Plane.glb";
		// const char* groundModelFilename = "assets/Mountain.glb";
		if (!gltf::LoadGLTF(groundModelFilename, /*scale=*/1.f, &meshes, &materials, &textures,
		                    debugOutput))
			return 1;
		for (const gltf::Mesh<float>& mesh : meshes)
			BulletMeshFromGltfMesh(mesh, physicsWorld);
	}

	////////////////////////////////////////////////////////////////////////////////
	// Game loop
	//

	// A made up but sane first frame
	float previousFrameTime = 0.016f;
	timer frameTimer;
	frameTimer.start();

	Camera cam(mainWindow);
	// bool useChaseCam = true;
	bool useChaseCam = false;

	mainWindow.shouldClear(false);

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		mainWindow.getBase()->setActive(true);

		processInput(input, vehicle);

		vehicle.Update(previousFrameTime);
		physicsWorld.Update(previousFrameTime);

		// Camera
		{
			if (!useChaseCam)
				cam.FreeCam(input, previousFrameTime);
			cam.UpdateStart();
			// Use vehicle transform to position camera
			if (useChaseCam)
			{
				const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
				btTransform camTransform = vehicleTransform.inverse();
				btScalar vehicleMat[16];
				// vehicleTransform.getOpenGLMatrix(vehicleMat);
				camTransform.getOpenGLMatrix(vehicleMat);
				// h3dSetNodeTransform(hordeCam, 0, 20, 0, /*rotationEuler=*/0, 0, 0, /*scaling=*/1,
				// 1, 1);

				// TODO: There's definitely something wrong with this function
				cam.ChaseCamera(vehicleMat);
			}
			cam.UpdateEnd();
		}

		// Place buggy model at vehicle position
		{
			const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
			float vehicleFloatMat[16];
			hordeMatrixFromBulletTransform(vehicleTransform, vehicleFloatMat);
			h3dSetNodeTransMat(buggyNode, vehicleFloatMat);
			// First person camera
			// h3dSetNodeTransMat(hordeCamera, vehicleFloatMat);
		}

		// glCallList(groundCallList);

		hordeUpdate(previousFrameTime);

		// Draw debug things (must happen AFTER h3dFinalizeFrame() but BEFORE swapping buffers)
		{
			// From http://www.horde3d.org/forums/viewtopic.php?f=1&t=978
			const float* cameraTranslationMat = 0;
			// Retrieve camera position...
			h3dGetNodeTransMats(hordeCamera, 0, &cameraTranslationMat);

			// In case of an invalid camera (e.g. pipeline not set) return
			if (cameraTranslationMat)
			{
				// ... and projection matrix
				float projectionMat[16];
				h3dGetCameraProjMat(hordeCamera, projectionMat);

				// ...

				// Set projection matrix
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(projectionMat);
				// apply camera transformation
				glMatrixMode(GL_MODELVIEW);
				float inverseCameraMat[16];
				macoyGluInvertMatrix(cameraTranslationMat, inverseCameraMat);
				// linalg::aliases::float4x4* transMat =
				// (linalg::aliases::float4x4*)&cameraTranslationMat;
				// linalg::aliases::float4x4 inverseCameraMat = linalg::inverse(*transMat);
				// glLoadMatrixf(&inverseCameraMat[0][0]);
				glLoadMatrixf(inverseCameraMat);

				// then later in e.g. drawGizmo

				// Uncomment for local transform, if necessary
				// glPushMatrix();
				// glMultMatrixf(nodeTransform);  // Load scene node matrix

				// ... draw code
				physicsWorld.DebugRender();

				// glPopMatrix();
			}
		}

		// Finished physics update and drawing; send it on its way
		mainWindow.update();

		mainWindow.getBase()->setActive(false);

		previousFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	hordeRelease();

	return 0;
}
