#include "Audio.hpp"

#include "Utilities.hpp"
#include "DebugDisplay.hpp"
#include "PhysicsVehicle.hpp"

#include "btBulletDynamicsCommon.h"
#include "graphics/graphics.hpp"
#include "sound/sound.hpp"

// For abs
#include "glm/common.hpp"

// #include <SFML/Sound.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <sstream>

#include <limits>
#include <stdlib.h> // rand
#include <cmath> // sin

sound sfxVehicleStartup;
sound sfxVehicleIdle;
sound sfxVehicleAccelerate;
sound sfxVehicleShifting;
sound sfxVehicleShutDown;
sound sfxVehicleChokeOut;
sound sfxVehicleSuspensionLanding;
sound sfxVehicleSuspensionJump;
sound sfxVehicleRoadNoiseDirt;
sound sfxVehicleSkidDirt;
sound sfxVehicleLanding;
sound sfxVehicleGasUp;
sound sfxVehicleCollision;
sound sfxVehicleBottomOut;
sound sfxVehicleRoll;
sound sfxVehicleBodySlide;

sound sfxWorldWindAtSpeed;
sound sfxWorldWindStationary;
sound sfxWorldSpaciousness;

sound sfxObjectiveGet;

listener audioListener;

enum class SoundLocation : int
{
	None = 0,

	Vehicle,
	World
};

struct SoundEffect
{
	const char* filename;
	sound* soundFx;
	SoundLocation location;
};

static SoundEffect soundFxFilePairs[] = {
    {"audio/MouthFX_VehicleStartup.ogg", &sfxVehicleStartup, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleIdle.ogg", &sfxVehicleIdle, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleAcceleration.ogg", &sfxVehicleAccelerate, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleShifting.ogg", &sfxVehicleShifting, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleShutDown.ogg", &sfxVehicleShutDown, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleChokeOut.ogg", &sfxVehicleChokeOut, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleSuspension.ogg", &sfxVehicleSuspensionLanding, SoundLocation::Vehicle},
    // {"audio/MouthFX_VehicleSuspensionJump.ogg", &sfxVehicleSuspensionJump,
    // SoundLocation::Vehicle},
    // {"audio/MouthFX_VehicleRoadNoiseDirt.ogg", &sfxVehicleRoadNoiseDirt, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleSkidDirt.ogg", &sfxVehicleSkidDirt, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleLanding.ogg", &sfxVehicleLanding, SoundLocation::Vehicle},
    // {"audio/MouthFX_VehicleGasUp.ogg", &sfxVehicleGasUp, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleCollision.ogg", &sfxVehicleCollision, SoundLocation::Vehicle},
    // {"audio/MouthFX_VehicleBottomOut.ogg", &sfxVehicleBottomOut, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleRoll.ogg", &sfxVehicleRoll, SoundLocation::Vehicle},
    {"audio/MouthFX_VehicleBodySlide.ogg", &sfxVehicleBodySlide, SoundLocation::Vehicle},

    {"audio/MouthFX_WorldWindAtSpeed.ogg", &sfxWorldWindAtSpeed, SoundLocation::World},
    {"audio/MouthFX_WorldWindIdle.ogg", &sfxWorldWindStationary, SoundLocation::World},
    {"audio/MouthFX_WorldSpaciousness.ogg", &sfxWorldSpaciousness, SoundLocation::World},

    {"audio/MouthFX_ObjectiveGet.ogg", &sfxObjectiveGet, SoundLocation::World},
};

void loadAudio()
{
	for (size_t i = 0; i < ArraySize(soundFxFilePairs); ++i)
	{
		if (!soundFxFilePairs[i].soundFx->load(soundFxFilePairs[i].filename))
			std::cout << "Failed to load " << soundFxFilePairs[i].filename << "\n";
	}
}

void debugPrintAudio()
{
	for (size_t i = 0; i < ArraySize(soundFxFilePairs); ++i)
	{
		std::ostringstream outputString;

		sf::Sound* baseSound = soundFxFilePairs[i].soundFx->getBase();
		switch (baseSound->getStatus())
		{
			case sf::Sound::Status::Stopped:
				break;
			case sf::Sound::Status::Paused:
				break;
			case sf::Sound::Status::Playing:
				outputString << soundFxFilePairs[i].filename << " Playing at volume "
				             << baseSound->getVolume();
				DebugDisplay::print(outputString.str());
				break;
		}
	}
}

typedef short SoundSample;
class BrownianNoiseAudioStream : public sf::SoundStream
{
public:
	void initializeNoiseStream()
	{
		// TODO: Properly zero
		for (unsigned int i = 0; i < ArraySize(sampleBuffer); ++i)
			sampleBuffer[i] = 0;
		unsigned int channelCount = 1;
		unsigned int sampleRate = 44100;
		initialize(channelCount, sampleRate);
	}

private:
	SoundSample sampleBuffer[100];
	virtual bool onGetData(sf::SoundStream::Chunk& data)
	{
		const SoundSample maxMotion = 10;
		const float pi = std::acos(-1);
		SoundSample lastSample = sampleBuffer[ArraySize(sampleBuffer) - 1];
		int triangleWaveDirection = 1;
		int triangleWaveSpeed = 1000;
		for (unsigned int i = 0; i < ArraySize(sampleBuffer); ++i)
		{
			// TODO Overflow protection
			// sampleBuffer[i] = lastSample + ((rand() % maxMotion) - maxMotion / 2);
			
			// Sawtooth wave
			// sampleBuffer[i] = ((i * 7) / (float)ArraySize(sampleBuffer)) * std::numeric_limits<short>::max();

			// Square wave
			// if (i < ArraySize(sampleBuffer) / 2)
			// 	sampleBuffer[i] = std::numeric_limits<short>::max();
			// else
			// 	sampleBuffer[i] = std::numeric_limits<short>::min();

			// Triangle wave
			// if (i == 0)
			// 	sampleBuffer[i] = std::numeric_limits<short>::min();
			// else if (sampleBuffer[i - 1] <= std::numeric_limits<short>::min() + triangleWaveSpeed)
			// 	triangleWaveDirection = 1;
			// else if (sampleBuffer[i - 1] >= std::numeric_limits<short>::max() - triangleWaveSpeed)
			// 	triangleWaveDirection = -1;
			// if (i > 0)
			// 	sampleBuffer[i] = sampleBuffer[i - 1] + (triangleWaveDirection * triangleWaveSpeed);

			// Sine wave - Not working
			sampleBuffer[i] = std::sin((i / (float)ArraySize(sampleBuffer)) * pi * 2) * (std::numeric_limits<short>::max() / 2);
			// if (i < 10)
				// std::cout << sampleBuffer[i] << "\n";
			
			lastSample = sampleBuffer[i];
		}

		data.sampleCount = ArraySize(sampleBuffer);
		data.samples = sampleBuffer;
		
		// Keep playing forever
		return true;
	}

	virtual void onSeek(sf::Time timeOffset)
	{
	}
};

void updateAudio(PhysicsVehicle& vehicle, float frameTime)
{
	// if (false)
	{
		static BrownianNoiseAudioStream noiseStream;
		static bool initialized = false;
		if (!initialized)
		{
			noiseStream.initializeNoiseStream();
			initialized = true;
		}
		noiseStream.play();
	}

	const glm::vec3 vehiclePosition = vehicle.GetPosition();
	audioListener.setPosition(vehiclePosition[0], vehiclePosition[1], vehiclePosition[2]);

	// Temporarily disable audio
	// return;

	static bool worldAudioStarted = false;
	if (!worldAudioStarted)
	{
		sfxWorldSpaciousness.loop(true);
		sfxWorldSpaciousness.play();

		sfxWorldWindStationary.loop(true);
		sfxWorldWindStationary.play();

		// Always startup car at the beginning
		sfxVehicleStartup.play();

		// sfxWorldWorldWind.loop(true);
		// sfxWorldWorldWind.play();

		worldAudioStarted = true;
	}

	// Update sound positions
	for (size_t i = 0; i < sizeof(soundFxFilePairs) / sizeof(SoundEffect); ++i)
	{
		switch (soundFxFilePairs[i].location)
		{
			case SoundLocation::World:
				soundFxFilePairs[i].soundFx->setPosition(vehiclePosition[0], vehiclePosition[1],
				                                         vehiclePosition[2]);
				break;
			case SoundLocation::Vehicle:
				soundFxFilePairs[i].soundFx->setPosition(vehiclePosition[0], vehiclePosition[1],
				                                         vehiclePosition[2]);
				break;
			default:
				break;
		}
	}

	// Vehicle
	{
		// Wheels
		for (int i = 0; i < vehicle.vehicle->getNumWheels(); i++)
		{
			const btWheelInfo& wheelInfo = vehicle.vehicle->getWheelInfo(i);
			// std::cout << "Wheel " << i << " skid " << wheelInfo.m_skidInfo << "\n";

			if (wheelInfo.m_skidInfo < 1.f)
				sfxVehicleSkidDirt.play();
		}

		// Engine noise
		// TODO This is bad
		static float inEngineStateForSeconds = 0.f;
		static bool accelerating = false;
		if (glm::abs(vehicle.ThrottlePercent) > 0.1f)
		{
			if (!accelerating)
			{
				accelerating = true;
				inEngineStateForSeconds = 0;
				sfxVehicleIdle.stop();
				sfxVehicleAccelerate.loop(true);
				sfxVehicleAccelerate.play();
			}

			inEngineStateForSeconds += frameTime;
		}
		else
		{
			if (accelerating)
			{
				accelerating = false;
				inEngineStateForSeconds = 0;
				sfxVehicleAccelerate.stop();
				sfxVehicleIdle.loop(true);
				sfxVehicleIdle.play();
			}
			inEngineStateForSeconds += frameTime;
		}

		// const btRigidBody* getRigidBody
	}
}

void playObjectiveGet()
{
	sfxObjectiveGet.play();	
}

void playVehicleShifting()
{
	sfxVehicleShifting.play();	
}
