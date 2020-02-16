#pragma once

void listConnectedJoysticks();

class PhysicsVehicle;
void processVehicleInputJoystick(PhysicsVehicle& vehicle, float frameTime);

struct Camera;
void handleCameraInput(Camera& camera, float frameTime);

