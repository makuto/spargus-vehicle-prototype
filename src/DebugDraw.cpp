#include "DebugDraw.hpp"

#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

void DebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& fromColor,
                         const btVector3& toColor)
{
	glBegin(GL_LINES);
	glColor3d(fromColor.getX(), fromColor.getY(), fromColor.getZ());
	glVertex3d(from.getX(), from.getY(), from.getZ());
	glColor3d(toColor.getX(), toColor.getY(), toColor.getZ());
	glVertex3d(to.getX(), to.getY(), to.getZ());
	glEnd();
}

void DebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	drawLine(from, to, color, color);
}

void DebugDraw::drawSphere(const btVector3& p, btScalar radius, const btVector3& color)
{
	glColor3d(color.getX(), color.getY(), color.getZ());
	int slices = 16;
	int stacks = 16;
	GLUquadric* sphereQuadric = gluNewQuadric();
	gluSphere(sphereQuadric, radius, slices, stacks);
	// TODO Position it!
	// p.getX(), p.getY(), p.getZ()
}

void DebugDraw::drawBox(const btVector3& bbMin, const btVector3& bbMax, const btVector3& color)
{
	glColor3d(color.getX(), color.getY(), color.getZ());
	// TODO
	// glDrawStrokedCube(AxisAlignedBox3f(Vec3f(bbMin.getX(), bbMin.getY(), bbMin.getZ()),
	// Vec3f(bbMax.getX(), bbMax.getY(), bbMax.getZ())));
}

void DebugDraw::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,
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

void DebugDraw::reportErrorWarning(const char* warningString)
{
	std::cout << warningString << std::endl;
}

void DebugDraw::draw3dText(const btVector3& location, const char* textString)
{
	// TODO
	// TextLayout textDraw;
	// textDraw.clear((0, 0, 0, 0));
	// textDraw.setColor(1, 1, 1));
	// textDraw.setFont(Font("Arial", 16));
	// textDraw.addCenteredLine(textString);
	// glDraw(glTexture(textDraw.render()), Vec2f(location.getX(), location.getY()));
}

void DebugDraw::setDebugMode(int newDebugMode)
{
	debugMode = newDebugMode;
}

int DebugDraw::getDebugMode() const
{
	return debugMode;
}
