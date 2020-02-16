#pragma once

#include <mutex>
#include <thread>

#include <glm/mat4x4.hpp>           // mat4
#include "sound/sound.hpp"

void loadAudio();

void debugPrintAudio();

typedef short SoundSample;
class VehicleEngineAudioStream : public sf::SoundStream
{
public:
	void initializeEngineAudio();

	void setEngineRpm(float newRpm);

private:
	std::mutex engineRpmMutex;
	float lastEngineRpm;

	SoundSample sampleBuffer[100];
	unsigned int bufferOffset;
	unsigned int sampleRate;

	virtual bool onGetData(sf::SoundStream::Chunk& data);

	virtual void onSeek(sf::Time timeOffset);
};

void updateAudio(const glm::mat4& mainListenerTransform, float frameTime);
class PhysicsVehicle;
void updateVehicleAudio(PhysicsVehicle& vehicle, float frameTime);

// TODO Kill these
void playObjectiveGet();
void playVehicleShifting();
