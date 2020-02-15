#include <iostream>

#include "graphics/graphics.hpp"
#include "input/input.hpp"
#include "timer/timer.hpp"

#include <GL/glew.h>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>          // distance2
#include <glm/gtx/vector_angle.hpp>  // angle
#include <glm/mat4x4.hpp>            // mat4

#include <map>
#include <sstream>

#include "Audio.hpp"
#include "Camera.hpp"
#include "Color.hpp"
#include "DebugDisplay.hpp"
#include "GraphicsInterface.hpp"
#include "Joystick.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "ModelUtilities/ModelLoader.hpp"
#include "ModelUtilities/ModelToBullet.hpp"
#include "ModelUtilities/ObjLoader.hpp"
#include "Performance.hpp"
#include "PhysicsVehicle.hpp"
#include "PhysicsWorld.hpp"
#include "PickUpObjective.hpp"
#include "Terrain.hpp"

// Window variables
// int WindowWidth = 1200;
// int WindowHeight = 700;
int WindowWidth = 1920;
int WindowHeight = 1080;
#define WIN_BACKGROUND_COLOR 20, 20, 20, 255

void LogOutput_WithPerfOutput(const Logging::Record& record)
{
	static char funcNameBuffer[256];
	Logging::FormatFuncName(funcNameBuffer, record.Function, sizeof(funcNameBuffer));
	std::ostringstream outputStream;
	outputStream << funcNameBuffer << "(): " << record.OutBuffer;
	std::cout << outputStream.str() << "\n";
	// TODO This is weird and probably wrong
	PerfLog(outputStream.str().c_str(), outputStream.str().size() + 1);
}

// Only use the slower logger if we're actually profiling
#ifdef PERF_ENABLE
Logging::Logger globalLogger(Logging::Severity::verbose, LogOutput_WithPerfOutput);
#else
Logging::Logger globalLogger(Logging::Severity::verbose, Logging::MinimalLogOutput);
#endif

void initializeWindow(window& win)
{
	{
		sf::ContextSettings settings = win.getBase()->getSettings();

		LOGI << "depth bits:" << settings.depthBits;
		LOGI << "stencil bits:" << settings.stencilBits;
		LOGI << "antialiasing level:" << settings.antialiasingLevel;
		LOGI << "version:" << settings.majorVersion << "." << settings.minorVersion;
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
	// glViewport(0, 0, width, height);

	Graphics::OnWindowResized(width, height);
}

void processVehicleInputKeyboard(inputManager& input, PhysicsVehicle& vehicle)
{
	bool useGameSteering = true;
	// Reset steering and forces immediately
	if (useGameSteering)
	{
		vehicle.ThrottlePercent = 0.f;
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
		vehicle.ThrottlePercent = 1.f;
		vehicle.BrakingForce = 0.f;
	}
	else if (input.isPressed(inputCode::Down))
	{
		// Start going backwards if the brake is held near zero
		if (speedKmHour < 0.1f)
		{
			vehicle.ThrottlePercent = -1.f;
			vehicle.BrakingForce = 0.f;
		}
		else
		{
			vehicle.ThrottlePercent = 0.f;
			vehicle.BrakingForce = vehicle.maxBrakingForce;
		}
	}
	else if (speedKmHour < 0.1f)
	{
		// Auto apply brakes
		vehicle.ThrottlePercent = 0.f;
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

bool useChaseCam = true;
// bool useChaseCam = false;

// bool debugPhysicsDraw = true;
bool debugPhysicsDraw = false;

bool debugDraw3D = true;
// bool debugDraw3D = false;
bool debugDraw2D = true;
// bool debugDraw2D = false;

bool useJoystick = true;
// bool useJoystick = false;

bool splitScreen = true;

float timeStepScale = 1.f;

void handleConfigurationInput(inputManager& input, PhysicsVehicle& mainVehicle)
{
	bool noKeyRepeat = false;

	if (input.WasTapped(inputCode::F1, noKeyRepeat))
		mainVehicle.Reset();
	if (input.WasTapped(inputCode::F2, noKeyRepeat))
		useChaseCam = !useChaseCam;
	if (input.WasTapped(inputCode::F3, noKeyRepeat))
		useJoystick = !useJoystick;
	if (input.WasTapped(inputCode::F4, noKeyRepeat))
		debugPhysicsDraw = !debugPhysicsDraw;
	if (input.WasTapped(inputCode::F9, noKeyRepeat))
	{
		debugDraw3D = !debugDraw3D;
		debugDraw2D = !debugDraw2D;
	}
	if (input.WasTapped(inputCode::F10, noKeyRepeat))
	{
		splitScreen = !splitScreen;
		if (!splitScreen)
		{
			// Make sure the viewport gets resized to normal
			Graphics::SetViewport(0, 0, WindowWidth, WindowHeight);
		}
	}

	// Time scaling
	if (input.WasTapped(inputCode::F5, true))
		timeStepScale -= 0.1f;
	if (input.WasTapped(inputCode::F6, true))
		timeStepScale += 0.1f;
	if (input.WasTapped(inputCode::F7, noKeyRepeat))
		timeStepScale = 1.f;
	if (input.WasTapped(inputCode::F8, noKeyRepeat))
		timeStepScale = 0.f;

	if (timeStepScale < 0.f)
		timeStepScale = 0.f;
}

int main()
{
	PerfTimeNamedScope(mainScope, "Main", tracy::Color::Gray);

	LOGI << "Spargus Vehicle Prototype";

	////////////////////////////////////////////////////////////////////////////////
	// Initialization
	//

	// Window/rendering
	PerfManualZoneBegin(MainWindowContext, "Main window initialization", tracy::Color::RoyalBlue);
	window mainWindow(WindowWidth, WindowHeight, "Spargus Vehicle Prototype", &windowResizeCB);
	PerfManualZoneEnd(MainWindowContext);
	{
		PerfTimeNamedScope(windowInit, "Initialize Graphics", tracy::Color::Navy);

		initializeWindow(mainWindow);
		DebugDisplay::initialize(&mainWindow);
		Graphics::Initialize(WindowWidth, WindowHeight);
	}

	inputManager input(&mainWindow);

	// World/meshes
	PhysicsWorld physicsWorld;
	PhysicsVehicle vehicle(physicsWorld);
	PhysicsVehicle otherVehicle(physicsWorld);
	Graphics::Object worldRender;
	{
		{
			PerfTimeNamedScope(worldInit, "World initialization", tracy::Color::MediumPurple);
			// Drawing the world
			worldRender.Initialize("World");
			// worldRender.SetTransform(glm::translate(glm::mat4(1.f), {0.f, -100.f, 0.f}));
			// World collision
			objToBulletTriangleMesh(physicsWorld, "Collision/World.obj");
		}

		// Terrain
		{
			createCollisionHeightfield(physicsWorld);
		}

		// objTest();
		// objToBulletTriangleMesh(physicsWorld, "Collision/Plane.obj");

		// Test bullet serialization
		if (false)
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
	}

	PickUpObjectives::Initialize(&physicsWorld);

	loadAudio();

	////////////////////////////////////////////////////////////////////////////////
	// Game loop
	//

	// A made up but sane first frame
	float previousFrameTime = 0.016f;
	timer frameTimer;
	frameTimer.start();

	Camera camera(mainWindow);

	mainWindow.shouldClear(false);

	while (!mainWindow.shouldClose() && !input.isPressed(inputCode::Escape))
	{
		PerfTimeNamedScope(timeFrameScope, "Frame", tracy::Color::SlateBlue);

		// Input
		{
			PerfTimeNamedScope(inputScope, "Input", tracy::Color::Sienna);

			handleConfigurationInput(input, vehicle);

			handleCameraInput(camera, previousFrameTime);

			if (useJoystick)
				processVehicleInputJoystick(vehicle, previousFrameTime);
			else
				processVehicleInputKeyboard(input, vehicle);
		}

		// Physics
		{
			PerfTimeNamedScope(physicsScope, "Physics", tracy::Color::Firebrick);

			// Vehicle updates
			{
				PerfTimeNamedScope(physicsVehicleScope, "Physics Vehicles",
				                   tracy::Color::OrangeRed);

				// Ricky Suicide
				{
					if (glm::distance2(otherVehicle.GetPosition(), vehicle.GetPosition()) <
					    glm::pow(10.f, 2))
					{
						// Stop near player
						otherVehicle.ThrottlePercent = 0.f;
						otherVehicle.BrakingForce = otherVehicle.maxBrakingForce;
					}
					else
					{
						otherVehicle.ThrottlePercent = 1.f;
						// Steer towards player
						glm::vec3 playerDelta =
						    glm::normalize(vehicle.GetPosition() - otherVehicle.GetPosition());
						glm::vec4 vehicleLeft4d =
						    otherVehicle.GetTransform() * glm::vec4(LeftAxis, 0.f);
						glm::vec3 vehicleLeft = glm::normalize(glm::vec3(vehicleLeft4d));
						float targetDot = glm::dot(vehicleLeft, playerDelta);
						float angleToPlayer = glm::angle(vehicleLeft, playerDelta);
						LOGV << glm::degrees(angleToPlayer) << " angle; dot " << targetDot;
						if (targetDot > 0.f)
							otherVehicle.VehicleSteering = otherVehicle.steeringClamp;
						else if (glm::abs(targetDot) < 0.1f)
							otherVehicle.VehicleSteering = 0.f;
						else
							otherVehicle.VehicleSteering = -otherVehicle.steeringClamp;
						DebugDraw::addLine(otherVehicle.GetPosition(), vehicle.GetPosition(),
						                   Color::Orange, Color::Blue,
						                   DebugDraw::Lifetime_OneFrame);
						DebugDraw::addLine(otherVehicle.GetPosition(),
						                   otherVehicle.GetPosition() + vehicleLeft, Color::Orange,
						                   Color::Blue, DebugDraw::Lifetime_OneFrame);
					}
				}

				const std::lock_guard<std::mutex> lock(g_vehiclesMutex);
				for (PhysicsVehicle* currentVehicle : g_vehicles)
				{
					// Make sure vehicle isn't falling through the world
					if (currentVehicle->GetPosition()[1] < -20.f)
					{
						// TODO: Make this put you back in the last known good position?
						currentVehicle->Reset();
					}
					currentVehicle->Update(previousFrameTime * timeStepScale);
				}
			}

			{
				PerfTimeNamedScope(physicsWorldScope, "Physics World", tracy::Color::Red);

				physicsWorld.Update(previousFrameTime * timeStepScale);
			}
		}

		PickUpObjectives::Update(previousFrameTime);

		// Audio
		{
			updateAudio(vehicle, previousFrameTime);
		}

		// Camera
		{
			if (!useChaseCam)
				camera.FreeCam(input, previousFrameTime);
			camera.UpdateStart();
			// Use vehicle transform to position camera
			if (useChaseCam)
			{
				glm::mat4 vehicleTransform = vehicle.GetTransform();
				camera.ChaseCamera(vehicleTransform);
			}
			camera.UpdateEnd();

			// Debug lines for camera
			{
				glm::vec3 scaledWorldCameraTargetDirection = camera.targetCameraDirection * 3.f;
				glm::vec3 vehiclePosition = vehicle.GetPosition();
				scaledWorldCameraTargetDirection += vehiclePosition;
				DebugDraw::addLine(vehiclePosition, scaledWorldCameraTargetDirection, Color::Blue,
				                   Color::Blue, DebugDraw::Lifetime_OneFrame);

				glm::vec4 vehicleFacingIntermediate(0.f, 0.f, 3.f, 1.f);
				vehicleFacingIntermediate = vehicle.GetTransform() * vehicleFacingIntermediate;
				glm::vec3 vehicleFacing(vehicleFacingIntermediate);
				DebugDraw::addLine(vehicle.GetPosition(), vehicleFacing, Color::Green, Color::Green,
				                   DebugDraw::Lifetime_OneFrame);
			}
		}

		// Rendering
		{
			PerfTimeNamedScope(renderingScope, "Rendering", tracy::Color::Yellow1);

			// glCallList(groundCallList);

			if (!splitScreen)
			{
				PerfTimeNamedScope(singleViewportScope, "Render single viewport",
				                   tracy::Color::Goldenrod1);

				Graphics::Update(previousFrameTime);
			}
			else
			{
				PerfTimeNamedScope(singleViewportScope, "Render splitscreen viewport",
				                   tracy::Color::DarkGoldenrod3);

				float windowHalfHeight = WindowHeight / 2.f;
				// Draw top screen
				Graphics::SetViewport(0, 0, WindowWidth, windowHalfHeight);
				Graphics::Update(previousFrameTime);

				// Draw bottom screen
				{
					glm::mat4 vehicleTransform = otherVehicle.GetTransform();
					camera.ChaseCamera(vehicleTransform);

					Graphics::SetViewport(0, windowHalfHeight, WindowWidth, windowHalfHeight);
					Graphics::Update(previousFrameTime);
				}
			}

			// Draw debug things (must happen AFTER h3dFinalizeFrame() but BEFORE swapping buffers)
			// From http://www.horde3d.org/forums/viewtopic.php?f=1&t=978
			if (debugDraw3D)
			{
				PerfTimeNamedScope(draw3DScope, "Render Debug 3D", tracy::Color::Orange1);

				glm::mat4 projectionMatrix = Graphics::GetCameraProjectionMatrixCopy();
				glMatrixMode(GL_PROJECTION);
				glLoadMatrixf(glmMatrixToHordeMatrixRef(projectionMatrix));

				// Apply camera transformation
				glMatrixMode(GL_MODELVIEW);
				glm::mat4 inverseCameraMat;
				glm::mat4 cameraMat = Graphics::GetCameraTransformCopy();
				inverseCameraMat = glm::inverse(cameraMat);
				glLoadMatrixf(glmMatrixToHordeMatrixRef(inverseCameraMat));

				// then later in e.g. drawGizmo
				// Uncomment for local transform, if necessary
				// glPushMatrix();
				// glMultMatrixf(nodeTransform);  // Load scene node matrix

				DebugDraw::render(previousFrameTime);

				if (debugPhysicsDraw)
				{
					PerfTimeNamedScope(drawPhysics3DScope, "Render Physics 3D",
					                   tracy::Color::Coral1);

					physicsWorld.DebugRender();
				}

				// If local transform, then
				// glPopMatrix();
			}
			else
			{
				// Make sure lines expire even when not being viewed
				DebugDraw::updateLifetimesOnly(previousFrameTime);
			}

			// 2D overlays
			if (debugDraw2D)
			{
				PerfTimeNamedScope(draw3DScope, "Render Debug 2D", tracy::Color::DeepPink1);

				// Time
				{
					std::ostringstream controls;
					controls << "F1 = reset vehicle "
					         << "F2 = toggle camera "
					         << "F3 = toggle input "
					         << "F4 = draw physics "
					         << "F9 = debug drawing "
					         << "F10 = toggle splitscreen";
					DebugDisplay::print(controls.str());

					std::ostringstream output;
					output << "Time scaling: " << timeStepScale << "x"
					       << " (F5 = -0.1x "
					       << "F6 = +0.1x "
					       << "F7 = 1x "
					       << "F8 = 0x)";
					DebugDisplay::print(output.str());
				}

				// Vehicle
				{
					btScalar speedKmHour = vehicle.vehicle->getCurrentSpeedKmHour();
					std::ostringstream output;
					output << "speedKmHour = " << speedKmHour
					       << " (mph = " << KilometersToMiles(speedKmHour)
					       << ") throttle = " << vehicle.ThrottlePercent * 100.f
					       << "% brake = " << vehicle.BrakingForce
					       << " gear = " << vehicle.SelectedGear << " rpm = " << vehicle.engineRpm
					       << "\n";
					DebugDisplay::print(output.str());

					for (int i = 0; i < vehicle.vehicle->getNumWheels(); i++)
					{
						const btWheelInfo& wheelInfo = vehicle.vehicle->getWheelInfo(i);
						// LOGD << "Wheel " << i << " skid " << wheelInfo.m_skidInfo << "\n";

						std::ostringstream outputSuspension;
						outputSuspension << "Wheel [" << i << "] skid " << wheelInfo.m_skidInfo
						                 << " suspension " << wheelInfo.m_wheelsSuspensionForce;
						DebugDisplay::print(outputSuspension.str());
					}

					const btTransform& vehicleTransform =
					    vehicle.vehicle->getChassisWorldTransform();
					btScalar vehicleMat[16];
					vehicleTransform.getOpenGLMatrix(vehicleMat);
					std::ostringstream outputPosition;
					outputPosition << "Vehicle position: " << vehicleMat[12] << ", "
					               << vehicleMat[13] << ", " << vehicleMat[14];
					DebugDisplay::print(outputPosition.str());
				}

				debugPrintAudio();

				// Required for 2D drawing
				mainWindow.getBase()->resetGLStates();
				{
					DebugDisplay::endFrame();
				}
			}
			else
			{
				// Make sure things don't pile up if not displaying normally
				DebugDisplay::clear();
			}

			mainWindow.getBase()->resetGLStates();
			{
				PickUpObjectives::RenderUI(mainWindow);
			}

			// Finished physics update and drawing; send it on its way
			{
				PerfTimeNamedScope(renderWindowScope, "Render window update",
				                   tracy::Color::DarkCyan);

				mainWindow.update();
			}
		}

		PerfEndFrame;

		previousFrameTime = frameTimer.getTime();
		frameTimer.start();
	}

	Graphics::Destroy();

	return 0;
}
