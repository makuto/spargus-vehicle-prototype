#include <iostream>

#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <GL/glew.h>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include "tiny_gltf.h"

#include <map>

#include "Camera.hpp"
#include "ModelLoader.hpp"
#include "ModelRender.hpp"
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

// This is an abomination
// From bullet3/examples/SoftDemo/SoftDemo.cpp (with modifications)
void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, btDiscreteDynamicsWorld* dynamicsWorld)
{
	size_t totalVerts = mesh.vertices.size() / 3;
	size_t totalTriangles = mesh.vertices.size() / 3 / 3;
	btCollisionShape* groundShape = nullptr;
	{
		// int i;
		// int j;

		// const int NUM_VERTS_X = 30;
		// const int NUM_VERTS_Y = 30;
		// const int totalVerts = NUM_VERTS_X * NUM_VERTS_Y;
		// const int totalTriangles = 2 * (NUM_VERTS_X - 1) * (NUM_VERTS_Y - 1);

		// // TODO wtf
		static btVector3* gGroundVertices = new btVector3[totalVerts];
		static int* gGroundIndices = new int[totalTriangles * 3];

		for (size_t i = 0; i < mesh.vertices.size(); i += 3)
		{
			for (size_t coord = 0; coord < 3; ++coord)
				gGroundVertices[i / 3][coord] = mesh.vertices[i + coord];
			gGroundIndices[i / 3] = i / 3;
		}

		// btScalar offset(-50);

		// for (i = 0; i < NUM_VERTS_X; i++)
		// {
		// 	for (j = 0; j < NUM_VERTS_Y; j++)
		// 	{
		// 		gGroundVertices[i + j * NUM_VERTS_X].setValue(
		// 		    (i - NUM_VERTS_X * 0.5f) * TRIANGLE_SIZE,
		// 		    // 0.f,
		// 		    waveheight * sinf((float)i) * cosf((float)j + offset),
		// 		    (j - NUM_VERTS_Y * 0.5f) * TRIANGLE_SIZE);
		// 	}
		// }

		int vertStride = sizeof(btVector3);
		int indexStride = 3 * sizeof(int);

		// int index = 0;
		// for (i = 0; i < NUM_VERTS_X - 1; i++)
		// {
		// 	for (int j = 0; j < NUM_VERTS_Y - 1; j++)
		// 	{
		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i + 1;
		// 		;

		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
		// 	}
		// }
		

		btTriangleIndexVertexArray* indexVertexArrays =
		    new btTriangleIndexVertexArray(totalTriangles, gGroundIndices, indexStride, totalVerts,
		                                   (btScalar*)&gGroundVertices[0].x(), vertStride);

		bool useQuantizedAabbCompression = true;

		groundShape = new btBvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);
		groundShape->setMargin(0.5);
	}

	btCollisionObject* newOb = new btCollisionObject();
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(0, 0, 0));
	newOb->setWorldTransform(tr);
	newOb->setInterpolationWorldTransform(tr);
	newOb->setCollisionShape(groundShape);
	dynamicsWorld->addCollisionObject(newOb);
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
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW\n";
		return 1;
	}

	inputManager input(&mainWindow);

	// Ground mesh from GLTF
	{
		// if (!LoadModelFromGltf("assets/World.glb", groundModel))
		std::vector<gltf::Mesh<float>> meshes;
		std::vector<gltf::Material> materials;
		std::vector<gltf::Texture> textures;
		bool debugOutput = false;
		if (!gltf::LoadGLTF("assets/World.glb", /*scale=*/1.f, &meshes, &materials, &textures,
		                    debugOutput))
			return 1;
		for (const gltf::Mesh<float>& mesh : meshes)
			BulletMeshFromGltfMesh(mesh, physicsWorld.world);
	}

	// initModel(groundModel);

	// GLCallListIndex groundModelCallList = buildCallListFromModel(groundModel);
	// GLCallListIndex groundModelCallList = buildCallListFromModel2(groundModel);

	// OpenGL world setup
	GLCallListIndex groundCallList = -1;
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
	bool useChaseCam = true;

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		if (!useChaseCam)
			cam.FreeCam(input);
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

		glCallList(groundCallList);
		// glCallList(groundModelCallList);

		physicsWorld.DebugRender();

		// drawModel2(groundModel);

		cam.UpdateEnd();

		// Finished physics update and drawing; send it on its way
		mainWindow.update();

		lastFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	return 0;
}
