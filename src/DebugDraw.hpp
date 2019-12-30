#pragma once

#include "LinearMath/btIDebugDraw.h"
#include "btBulletDynamicsCommon.h"

#include <glm/mat4x4.hpp>  // mat4
#include "Color.hpp"

class BulletDebugDraw : public btIDebugDraw
{
	int debugMode;

public:
	virtual ~BulletDebugDraw() = default;
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor,
	                      const btVector3& toColor);
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	virtual void drawSphere(const btVector3& p, btScalar radius, const btVector3& color);
	virtual void drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color);
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
	                              btScalar distance, int lifeTime, const btVector3& color);
	virtual void reportErrorWarning(const char* warningString);
	virtual void draw3dText(const btVector3& location, const char* textString);
	virtual void setDebugMode(int debugMode);
	virtual int getDebugMode() const;
};

namespace DebugDraw
{
void render(float frameTime);
void updateLifetimesOnly(float frameTime);

extern const float Lifetime_OneFrame;
void addLine(const glm::vec3& from, const glm::vec3& to, const Color4<float>& fromColor,
             const Color4<float>& toColor, float lifetimeSeconds);
}  // namespace DebugDraw
