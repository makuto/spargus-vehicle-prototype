#include "DebugDraw.hpp"

#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <iostream>
#include <vector>

#include "Logging.hpp"

namespace DebugDraw
{
void drawLine(const glm::vec3& from, const glm::vec3& to, const Color4<float>& fromColor,
              const Color4<float>& toColor)
{
	glBegin(GL_LINES);
	glColor3d(fromColor.components[0], fromColor.components[1], fromColor.components[2]);
	glVertex3d(from[0], from[1], from[2]);
	glColor3d(toColor.components[0], toColor.components[1], toColor.components[2]);
	glVertex3d(to[0], to[1], to[2]);
	glEnd();
}

struct Line
{
	glm::vec3 from;
	glm::vec3 to;
	Color4<float> fromColor;
	Color4<float> toColor;
	float lifetimeSeconds;
};

struct DebugDrawState
{
	std::vector<Line> lines;
};
DebugDrawState g_DebugDrawState;
const float Lifetime_OneFrame = 0.f;

void render(float frameTime)
{
	for (std::vector<Line>::iterator lineIt = g_DebugDrawState.lines.begin();
	     lineIt != g_DebugDrawState.lines.end();)
	{
		drawLine(lineIt->from, lineIt->to, lineIt->fromColor, lineIt->toColor);

		lineIt->lifetimeSeconds -= frameTime;
		if (lineIt->lifetimeSeconds < 0.f)
			lineIt = g_DebugDrawState.lines.erase(lineIt);
		else
			++lineIt;
	}
}

void updateLifetimesOnly(float frameTime)
{
	for (std::vector<Line>::iterator lineIt = g_DebugDrawState.lines.begin();
	     lineIt != g_DebugDrawState.lines.end();)
	{
		lineIt->lifetimeSeconds -= frameTime;
		if (lineIt->lifetimeSeconds < 0.f)
			lineIt = g_DebugDrawState.lines.erase(lineIt);
		else
			++lineIt;
	}
}

void addLine(const glm::vec3& from, const glm::vec3& to, const Color4<float>& fromColor,
             const Color4<float>& toColor, float lifetimeSeconds)
{
	Line newLine{from, to, fromColor, toColor, lifetimeSeconds};
	g_DebugDrawState.lines.push_back(newLine);
}
}  // namespace DebugDraw

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to,
                               const btVector3& fromColor, const btVector3& toColor)
{
	glBegin(GL_LINES);
	glColor3d(fromColor.getX(), fromColor.getY(), fromColor.getZ());
	glVertex3d(from.getX(), from.getY(), from.getZ());
	glColor3d(toColor.getX(), toColor.getY(), toColor.getZ());
	glVertex3d(to.getX(), to.getY(), to.getZ());
	glEnd();
}

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	drawLine(from, to, color, color);
}

void BulletDebugDraw::drawSphere(const btVector3& p, btScalar radius, const btVector3& color)
{
	glColor3d(color.getX(), color.getY(), color.getZ());
	int slices = 16;
	int stacks = 16;
	GLUquadric* sphereQuadric = gluNewQuadric();
	gluSphere(sphereQuadric, radius, slices, stacks);
	// TODO Position it!
	// p.getX(), p.getY(), p.getZ()
}

void BulletDebugDraw::drawBox(const btVector3& bbMin, const btVector3& bbMax,
                              const btVector3& color)
{
	glColor3d(color.getX(), color.getY(), color.getZ());
	// TODO
	// glDrawStrokedCube(AxisAlignedBox3f(Vec3f(bbMin.getX(), bbMin.getY(), bbMin.getZ()),
	// Vec3f(bbMax.getX(), bbMax.getY(), bbMax.getZ())));
}

void BulletDebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
                                       btScalar distance, int lifeTime, const btVector3& color)
{
	glColor3d(color.getX(), color.getY(), color.getZ());
	glBegin(GL_LINES);
	// From
	glVertex3d(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	// To
	glVertex3d(normalOnB.getX() + PointOnB.getX(), normalOnB.getY() + PointOnB.getY(),
	           normalOnB.getZ() + PointOnB.getZ());
	glEnd();
}

void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
	LOGW << warningString;
}

void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
	// TODO
	// TextLayout textDraw;
	// textDraw.clear((0, 0, 0, 0));
	// textDraw.setColor(1, 1, 1));
	// textDraw.setFont(Font("Arial", 16));
	// textDraw.addCenteredLine(textString);
	// glDraw(glTexture(textDraw.render()), Vec2f(location.getX(), location.getY()));
}

void BulletDebugDraw::setDebugMode(int newDebugMode)
{
	debugMode = newDebugMode;
}

int BulletDebugDraw::getDebugMode() const
{
	return debugMode;
}
