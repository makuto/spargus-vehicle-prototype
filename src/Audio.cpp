#include "Audio.hpp"

#include "DebugDisplay.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Performance.hpp"
#include "PhysicsVehicle.hpp"
#include "Utilities.hpp"

#include "btBulletDynamicsCommon.h"
#include "graphics/graphics.hpp"
#include "sound/sound.hpp"

// For abs
#include <glm/vec3.hpp>  // vec3
#include "glm/common.hpp"

// #include <SFML/Sound.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <sstream>

#include <stdlib.h>  // rand
#include <cmath>     // sin
#include <limits>

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
	PerfTimeNamedScope(loadAudioScope, "Load Audio", tracy::Color::RoyalBlue1);

	for (size_t i = 0; i < ArraySize(soundFxFilePairs); ++i)
	{
		PerfTimeNamedScope(loadAudioSingleFileScope, "Load Audio file", tracy::Color::RoyalBlue3);
		PerfSetNameFormat(loadAudioSingleFileScope, "Load Audio '%s'",
		                  soundFxFilePairs[i].filename);

		if (!soundFxFilePairs[i].soundFx->load(soundFxFilePairs[i].filename))
		{
			LOGE << "Failed to load " << soundFxFilePairs[i].filename << "\n";
			continue;
		}

		switch (soundFxFilePairs[i].location)
		{
			case SoundLocation::World:
				// From SFML docs:
				// Making a sound relative to the listener will ensure that it will always be played
				// the same way regardless of the position of the listener
				soundFxFilePairs[i].soundFx->getBase()->setRelativeToListener(true);
				break;
			default:
				break;
		}
	}

	// Disable audio
	// sf::Listener::setGlobalVolume(0.f);
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

void VehicleEngineAudioStream::initializeEngineAudio()
{
	// TODO: Properly zero
	for (unsigned int i = 0; i < ArraySize(sampleBuffer); ++i)
		sampleBuffer[i] = 0;

	bufferOffset = 0;
	unsigned int channelCount = 1;
	sampleRate = 44100;
	initialize(channelCount, sampleRate);

	lastEngineRpm = 0.f;
}

void VehicleEngineAudioStream::setEngineRpm(float newRpm)
{
	const std::lock_guard<std::mutex> lock(engineRpmMutex);
	lastEngineRpm = newRpm;
}

bool VehicleEngineAudioStream::onGetData(sf::SoundStream::Chunk& data)
{
	PerfTimeNamedScope(audioScope, "Audio: Engine stream", tracy::Color::RoyalBlue1);

	float engineRpm = 0.f;
	{
		const std::lock_guard<std::mutex> lock(engineRpmMutex);
		engineRpm = lastEngineRpm;
	}

	const SoundSample maxMotion = 10;
	const float pi = std::acos(-1);
	SoundSample lastSample = sampleBuffer[ArraySize(sampleBuffer) - 1];
	int triangleWaveDirection = 1;
	int triangleWaveSpeed = 1000;
	unsigned int engineUpSquareNextNSamples = 0;
	for (unsigned int i = 0; i < ArraySize(sampleBuffer); ++i)
	{
		// TODO Overflow protection
		// sampleBuffer[i] = lastSample + ((rand() % maxMotion) - maxMotion / 2);

		// Sawtooth wave
		// sampleBuffer[i] = ((i * 7) / (float)ArraySize(sampleBuffer)) *
		// std::numeric_limits<short>::max();

		// Square wave
		// if (i < ArraySize(sampleBuffer) / 2)
		// 	sampleBuffer[i] = std::numeric_limits<short>::max();
		// else
		// 	sampleBuffer[i] = std::numeric_limits<short>::min();

		// Triangle wave
		// if (i == 0)
		// 	sampleBuffer[i] = std::numeric_limits<short>::min();
		// else if (sampleBuffer[i - 1] <= std::numeric_limits<short>::min() +
		// triangleWaveSpeed) 	triangleWaveDirection = 1; else if (sampleBuffer[i - 1] >=
		// std::numeric_limits<short>::max() - triangleWaveSpeed) 	triangleWaveDirection = -1;
		// if (i > 0) 	sampleBuffer[i] = sampleBuffer[i - 1] + (triangleWaveDirection *
		// triangleWaveSpeed);

		// Sine wave - Not working
		// sampleBuffer[i] =
		//     std::sin(((i % ArraySize(sampleBuffer)) * pi * 2 * (engineRpm / 1000.f)) *
		//              (std::numeric_limits<short>::max() / 2));

		// Engine rpm
		if (i + bufferOffset % 100 == 0)
		// if (i + bufferOffset % (int)(sampleRate / (engineRpm / 60.f)) == 0)
		{
			engineUpSquareNextNSamples = 10;
		}

		if (engineUpSquareNextNSamples > 0)
		{
			engineUpSquareNextNSamples--;
			sampleBuffer[i] = std::numeric_limits<short>::max();
		}
		else
		{
			sampleBuffer[i] = std::numeric_limits<short>::min();
		}

		// if (i < 10)
		// LOGD << sampleBuffer[i];

		lastSample = sampleBuffer[i];
	}

	data.sampleCount = ArraySize(sampleBuffer);
	data.samples = sampleBuffer;

	bufferOffset += data.sampleCount;

	// Keep playing forever
	return true;
}

void VehicleEngineAudioStream::onSeek(sf::Time timeOffset)
{
}

// Support only one listener, even in splitscreen. I'm not sure how this is handled normally
// TODO: Add multiplayer 3D audio support (do some research)
void updateAudio(const glm::mat4& mainListenerTransform, float frameTime)
{
	PerfTimeNamedScope(globalAudioScope, "Global Audio", tracy::Color::Ivory4);

	audioListener.setPosition(mainListenerTransform[3][0], mainListenerTransform[3][1],
	                          mainListenerTransform[3][2]);
	// TODO Add to base2.0
	{
		glm::vec3 mainListenerDirection = RotateGlmVec3ByMat4(mainListenerTransform, ForwardAxis);
		sf::Listener::setDirection(mainListenerDirection[0], mainListenerDirection[1],
		                           mainListenerDirection[2]);

		glm::vec3 mainListenerUpAxis = RotateGlmVec3ByMat4(mainListenerTransform, UpAxis);
		sf::Listener::setUpVector(mainListenerUpAxis[0], mainListenerUpAxis[1],
		                          mainListenerUpAxis[2]);
	}

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
				// soundFxFilePairs[i].soundFx->setPosition(
				//     mainListenerPosition[0], mainListenerPosition[1], mainListenerPosition[2]);
				// This is handled by setRelativePosition(true)
				break;
			default:
				break;
		}
	}
}

// Currently not used
void updateVehicleAudio(PhysicsVehicle& vehicle, float frameTime)
{
	PerfTimeNamedScope(audioScope, "Vehice Audio", tracy::Color::Ivory4);

	const glm::vec3 vehiclePosition = vehicle.GetPosition();

	// Update sound positions
	// TODO: This won't work for multiple vehicles (each needs their own sound effects)
	for (size_t i = 0; i < sizeof(soundFxFilePairs) / sizeof(SoundEffect); ++i)
	{
		switch (soundFxFilePairs[i].location)
		{
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

		// Sample-based Engine noise
		if (false)
		{
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
