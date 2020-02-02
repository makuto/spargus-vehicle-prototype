#pragma once


void loadAudio();

void debugPrintAudio();

class PhysicsVehicle;
void updateAudio(PhysicsVehicle& vehicle, float frameTime);
// TODO Kill these
void playObjectiveGet();
void playVehicleShifting();
