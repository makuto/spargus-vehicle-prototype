#include "PickUpObjective.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>

#include <algorithm>
#include <sstream>
#include <vector>

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "timer/timer.hpp"

#include "Audio.hpp"
#include "DebugDisplay.hpp"
#include "GraphicsObject.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "PhysicsWorld.hpp"
#include "Utilities.hpp"

namespace PickUpObjectives
{
struct PickUpObjective
{
	bool objectiveHit;
	Graphics::Object renderObject;
	btRigidBody* physicsBody = nullptr;
};

struct
{
	std::vector<PickUpObjective> objectives;
	// This sucks
	PhysicsWorld* cachedPhysicsWorld = nullptr;

	timer objectiveTimer;
	
	float completionTime = 0.f;
	
	// UI
	text displayText;
	int numObjectivesHit = 0;
	int totalNumObjectives = 0;
} g_PickUpObjectivesState;

void onCollisionListener(const btRigidBody* body0, const btRigidBody* body1, CollisionState eState)
{
	// Ignore separations
	if (eState != CollisionState::NowColliding)
		return;

	// Find any objective, ignoring which body
	std::vector<PickUpObjective>::iterator findIt = std::find_if(
	    g_PickUpObjectivesState.objectives.begin(), g_PickUpObjectivesState.objectives.end(),
	    [body0, body1](const PickUpObjective& objective) {
		    return objective.physicsBody == body0 || objective.physicsBody == body1;
	    });

	// No objectives involved
	if (findIt == g_PickUpObjectivesState.objectives.end())
		return;
	else
		LOGD << "Hit objective";

	// Remove visible and collision parts of objective
	findIt->renderObject.Destroy();
	if (g_PickUpObjectivesState.cachedPhysicsWorld)
		g_PickUpObjectivesState.cachedPhysicsWorld->world->removeRigidBody(findIt->physicsBody);

	findIt->objectiveHit = true;
	playObjectiveGet();
}

void Initialize(PhysicsWorld* physicsWorld)
{
	g_PickUpObjectivesState.cachedPhysicsWorld = physicsWorld;

	g_PickUpObjectivesState.cachedPhysicsWorld->AddCollisionListener(onCollisionListener);

	// Blender coords to ours: swap Y and Z, then invert Z
	glm::vec3 objectivePositions[] = {
	    // Near start
	    glm::vec3(20.f, 0.f, 0.f),
	    // Ramps
	    glm::vec3(-39.91f, 13.25f, -214.2f),
	    // Left pinch
	    glm::vec3(125.2f, 39.62f, -66.95f),
	    // Box
	    glm::vec3(-78.82f, 15.01f, -6.5f),
	    // Hidden butt
	    glm::vec3(249.6f, 3.254f, -128.6f),
	    // Big hill jump
	    glm::vec3(-125.4f, 37.37f, -0.975f),
		// Loop-de-loop
		glm::vec3(134.8f, 24.24f, 212.3f),
	};

	// Objectives
	g_PickUpObjectivesState.objectives.resize(ArraySize(objectivePositions));

	for (unsigned int i = 0; i < ArraySize(objectivePositions); ++i)
	{
		glm::mat4 objectiveLocation = glm::translate(glm::mat4(1.f), objectivePositions[i]);
		g_PickUpObjectivesState.objectives[i].renderObject.SetTransform(objectiveLocation);
		g_PickUpObjectivesState.objectives[i].renderObject.Initialize("PickUp");
	}

	// Create bullet trigger bodies for g_PickUpObjectivesState.objectives
	for (PickUpObjective& objective : g_PickUpObjectivesState.objectives)
	{
		const float triggerHalfWidth = 3.15f;
		// TODO: Leak
		btCollisionShape* triggerShape =
		    new btBoxShape(btVector3(triggerHalfWidth, triggerHalfWidth, triggerHalfWidth));

		btTransform startTransform;
		startTransform.setIdentity();
		startTransform.setOrigin(glmVec3ToBulletVector(objective.renderObject.GetPosition()));
		objective.physicsBody = g_PickUpObjectivesState.cachedPhysicsWorld->localCreateRigidBody(
		    PhysicsWorld::StaticRigidBodyMass, startTransform, triggerShape);
		objective.physicsBody->setCollisionFlags(objective.physicsBody->getCollisionFlags() |
		                                         btRigidBody::CF_NO_CONTACT_RESPONSE);
		objective.physicsBody->setUserPointer((void*)&objective);
	}

	if (!g_PickUpObjectivesState.displayText.loadFont("data/fonts/UbuntuMono-R.ttf"))
	{
		LOGE << "Error: Cannot load default text font";
	}

	g_PickUpObjectivesState.objectiveTimer.start();
}

void Update(float frameTime)
{
	g_PickUpObjectivesState.numObjectivesHit = 0;
	for (const PickUpObjective& objective : g_PickUpObjectivesState.objectives)
	{
		if (objective.objectiveHit)
			++g_PickUpObjectivesState.numObjectivesHit;
	}

	g_PickUpObjectivesState.totalNumObjectives = g_PickUpObjectivesState.objectives.size();
}

void RenderUI(window& win)
{
	std::ostringstream objectiveStatus;

	if (g_PickUpObjectivesState.numObjectivesHit < g_PickUpObjectivesState.totalNumObjectives)
	{
		objectiveStatus << "Objective time: " << g_PickUpObjectivesState.objectiveTimer.getTime()
		                << " Objectives hit: " << g_PickUpObjectivesState.numObjectivesHit << "/"
		                << g_PickUpObjectivesState.totalNumObjectives;
	}
	else
	{
		if (!g_PickUpObjectivesState.completionTime)
			g_PickUpObjectivesState.completionTime =
			    g_PickUpObjectivesState.objectiveTimer.getTime();

		objectiveStatus << "Accomplished in " << g_PickUpObjectivesState.completionTime
		                << " seconds";
	}

	g_PickUpObjectivesState.displayText.setText(objectiveStatus.str());
	g_PickUpObjectivesState.displayText.setPosition(30.f, win.getHeight() - 100.f);
	win.draw(&g_PickUpObjectivesState.displayText);
}

}  // namespace PickUpObjectives
