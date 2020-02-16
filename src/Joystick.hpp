#pragma once

void listConnectedJoysticks();

class PhysicsVehicle;
void processVehicleInputJoystick(PhysicsVehicle& vehicle, float frameTime, int playerJoystickId);

struct Camera;
void handleCameraInput(Camera& camera, float frameTime, int playerJoystickId);

