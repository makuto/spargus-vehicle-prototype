#include "VehicleEditor.hpp"

#include "GameVehicle.hpp"
#include "Logging.hpp"
#include "PhysicsVehicle.hpp"

#include "imgui-SFML.h"
#include "imgui.h"

#include <glm/mat4x4.hpp>  // mat4

#include <fstream>
#include <string>

bool showTuningHelp = false;
static const char* tuningDocumentationFilename = "docs/VehicleTuning.org";
static std::vector<std::string> tuningDocumentationLines;

static void loadTuningDocumentation()
{
	std::ifstream inputFileStream(tuningDocumentationFilename);
	std::string line;
	if (inputFileStream.is_open())
	{
		while (std::getline(inputFileStream, line))
		{
			tuningDocumentationLines.push_back(line);
		}
	}
	else
		LOGE << "Could not open Vehicle tuning documentation '" << tuningDocumentationFilename
		     << "'";
}

// This currently has to delete and recreate the vehicle when parameters change
void updateVehicleEditor(PhysicsWorld& world, PhysicsVehicle** vehicleToEdit)
{
	PhysicsVehicleTuning& tuning = GameVehicles::defaultTuning;

	ImGui::Begin("Vehicle Tuning Editor");
	{
		ImGui::SliderFloat("Mass (Kg)", &tuning.massKg, 1.f, 10000.f);
		ImGui::SliderFloat("Chassis Width", &tuning.chassisWidth, 0.f, 50.f);
		ImGui::SliderFloat("Chassis Height", &tuning.chassisHeight, 0.f, 50.f);
		ImGui::SliderFloat("Chassis Length", &tuning.chassisLength, 0.f, 50.f);
		ImGui::SliderFloat("Connection Height", &tuning.connectionHeight, 0.f, 10.f);
		ImGui::SliderFloat("wheel Radius", &tuning.wheelRadius, 0.f, 10.f);
		ImGui::SliderFloat("wheel Width", &tuning.wheelWidth, 0.f, 10.f);
		ImGui::Separator();
		ImGui::SliderFloat("Wheel Friction", &tuning.wheelFriction, 0.f, 1000.f);
		ImGui::Separator();
		ImGui::SliderFloat("Suspension Rest Length", &tuning.suspensionRestLength, 0.f, 1000.f);
		ImGui::SliderFloat("Suspension Stiffness", &tuning.suspensionStiffness, 0.f, 1000.f);
		ImGui::SliderFloat("Suspension Damping Relaxation", &tuning.suspensionDampingRelaxation,
		                   0.f, 1000.f);
		ImGui::SliderFloat("Suspension Damping Compression", &tuning.suspensionDampingCompression,
		                   0.f, 1000.f);
		ImGui::SliderFloat("Roll Influence", &tuning.rollInfluence, 0.f, 1000.f);
		ImGui::Separator();
		ImGui::SliderFloat("Default Braking Force", &tuning.defaultBrakingForce, 0.f, 1000.f);

		ImGui::Separator();
		ImGui::Checkbox("Show tuning documentation (Warning: may affect framerate)", &showTuningHelp);
		if (ImGui::Button("Reset to defaults"))
			tuning = {};
		if (ImGui::Button("Commit changes and reset vehicle"))
		{
			// TODO No way to reset vehicle to new tuning
			*vehicleToEdit = GameVehicles::CreateVehicle(world, glm::mat4(1.f));
		}
	}
	ImGui::End();

	// Not great, but sort of works
	if (showTuningHelp)
	{
		if (tuningDocumentationLines.empty())
			loadTuningDocumentation();

		ImGui::Begin("Vehicle Tuning Documentation");

		bool headerShowLines = true;

		for (std::string& line : tuningDocumentationLines)
		{
			// Handle case where *bold text* starts a line
			if (line[0] == '*' && (line[1] == ' ' || line[1] == '*'))
				headerShowLines = ImGui::CollapsingHeader(line.c_str());
			else
			{
				if (!headerShowLines)
					continue;

				ImGui::TextWrapped("%s", line.c_str());
			}
		}

		ImGui::End();
	}
}
