#include "Audio.hpp"

#include "PhysicsVehicle.hpp"
#include "DebugDisplay.hpp"

#include "btBulletDynamicsCommon.h"
#include "sound/sound.hpp"
#include "graphics/graphics.hpp"

// #include <SFML/Sound.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <sstream>

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
};

void loadAudio()
{
	for (size_t i = 0; i < sizeof(soundFxFilePairs) / sizeof(SoundEffect); ++i)
	{
		if (!soundFxFilePairs[i].soundFx->load(soundFxFilePairs[i].filename))
			std::cout << "Failed to load " << soundFxFilePairs[i].filename << "\n";
	}
}

template <typename T,unsigned Length>
inline unsigned ArraySize(const T (&v)[Length]) { return Length; }

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

void updateAudio(PhysicsVehicle& vehicle)
{
	const btTransform& vehicleTransform = vehicle.vehicle->getChassisWorldTransform();
	const btVector3 vehiclePosition = vehicleTransform.getOrigin();
	audioListener.setPosition(vehiclePosition.getX(), vehiclePosition.getY(),
	                          vehiclePosition.getZ());

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
				soundFxFilePairs[i].soundFx->setPosition(
				    vehiclePosition.getX(), vehiclePosition.getY(), vehiclePosition.getZ());
				break;
			case SoundLocation::Vehicle:
				soundFxFilePairs[i].soundFx->setPosition(
				    vehiclePosition.getX(), vehiclePosition.getY(), vehiclePosition.getZ());
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
			
			std::ostringstream outputSuspension;
			outputSuspension << "Wheel [" << i << "] skid " << wheelInfo.m_skidInfo << " suspension " << wheelInfo.m_wheelsSuspensionForce;
			DebugDisplay::print(outputSuspension.str());
		}

		// Engine noise
		if (vehicle.EngineForce > 5.f)
		{
			sfxVehicleIdle.stop();
			sfxVehicleAccelerate.loop(true);
			sfxVehicleAccelerate.play();
		}
		else
		{
			sfxVehicleAccelerate.stop();
			sfxVehicleIdle.loop(true);
			sfxVehicleIdle.play();
		}

		// const btRigidBody* getRigidBody
	}
}
