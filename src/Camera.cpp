#include "Camera.hpp"

#include <cmath>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include "graphics/graphics.hpp"
#include "input/input.hpp"

Camera::Camera(window& winOwner) : win(winOwner)
{
	winBase = win.getBase();
	win.getBase()->setMouseCursorVisible(false);
	sf::Mouse::setPosition(sf::Vector2i(win.getWidth() / 2, win.getHeight() / 2), *winBase);
}

void Camera::FreeCam(inputManager& input)
{
	if (input.isPressed(inputCode::W))
	{
		/* yrotrad = (yrot / 180 * 3.141592654f);
		    xrotrad = (xrot / 180 * 3.141592654f);
		    xpos += float(sin(yrotrad)) ;
		    zpos -= float(cos(yrotrad)) ;
		 * */
		camTranslate[0] += float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -= float(cos(camRot[1] / 180 * 3.141592654));
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::S))
	{
		camTranslate[0] -= float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] += float(cos(camRot[1] / 180 * 3.141592654));
		// camTranslate[2]=50*win.GetFrameTime();
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::A))
	{
		camTranslate[0] -= float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -= float(sin(camRot[1] / 180 * 3.141592654));
	}
	if (input.isPressed(inputCode::D))
	{
		camTranslate[0] += float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] += float(sin(camRot[1] / 180 * 3.141592654));
	}

	// camRot[1]=(10*(in->GetMouseX()*win.GetWidth()/10) + 400)/win.GetWidth();
	camRot[1] += (sf::Mouse::getPosition(*winBase).x - prevX) / 5 /**winBase.GetWidth()/3600*/;
	camRot[0] += (sf::Mouse::getPosition(*winBase).y - prevY) / 5;
	prevX = sf::Mouse::getPosition(*winBase).x;
	prevY = sf::Mouse::getPosition(*winBase).y;
	if (prevX <= 100 || prevX >= 200)
	{
		sf::Mouse::setPosition(sf::Vector2i(winBase->getSize().x / 2, winBase->getSize().y / 2),
		                       *winBase);
		prevX = winBase->getSize().x / 2;
	}
	if (prevY <= 100 || prevY >= 200)
	{
		sf::Mouse::setPosition(sf::Vector2i(winBase->getSize().x / 2, winBase->getSize().y / 2),
		                       *winBase);
		prevY = win.getHeight() / 2;
	}
}

void Camera::UpdateStart()
{
	// Set camera
	if (camRot[0] < -90)
		camRot[0] = -90;
	if (camRot[0] > 100)
		camRot[0] = 100;
	// Make the degree positive
	if (camRot[1] < 0)
		camRot[1] += 360;
	// Clamp the degree to 0-360
	if (camRot[1] >= 360)
		camRot[1] -= 360;
	if (camRot[1] <= -360)
		camRot[1] += 360;
	// Calculate the slope of the movement vector
	/*camTranslate[0] *=sin(camRot[1]);
	camTranslate[2] *=cos(camRot[1]);*/
	// Add the translation vector
	camPos[0] += camTranslate[0];
	camPos[1] += camTranslate[1];
	camPos[2] += camTranslate[2];

	// Define the camera matrix
	glLoadIdentity();
	gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
	glRotatef(camRot[0], 1, 0, 0);
	glRotatef(camRot[1], 0, 1, 0);
	glRotatef(camRot[2], 0, 0, 1);

	glTranslatef(-camPos[0], camPos[1], -camPos[2]);
	glPushMatrix();
}

void Camera::ChaseCamera(double* openGlMatrix)
{
	const double cameraHeight = 15.0;
	const double cameraPullback = -15.0;
	const double cameraPitch = 30.0;

	// Undo old matrix
	glPopMatrix();

	glLoadIdentity();

	// Look at method
	// if (true)
	// {
	// 	// double position[] = {openGlMatrix[12], openGlMatrix[13] + cameraHeight,
	// 	// openGlMatrix[14]};
	// 	double position[] = {openGlMatrix[12], openGlMatrix[13] + cameraHeight,
	// 	                     0.};  // openGlMatrix[14] + cameraPullback};
	// 	// Offset position
	// 	// Look at target
	// 	double targetPosition[] = {openGlMatrix[12], openGlMatrix[13], openGlMatrix[14]};
	// 	// double targetPosition[] = {0., 0., 0.};
	// 	gluLookAt(
	// 	    /*Position=*/position[0], position[1], position[2],
	// 	    /*Look at (world space)=*/targetPosition[0], targetPosition[1], targetPosition[2],
	// 	    /*Up vector=*/0, 1, 0);
	// }

	// Look at method 2
	if (false)
	{
		// double position[] = {openGlMatrix[12], openGlMatrix[13] + cameraHeight,
		// openGlMatrix[14]};
		double position[] = {openGlMatrix[12], openGlMatrix[13], openGlMatrix[14]};
		// Offset position
		// Look at target
		double targetPosition[] = {openGlMatrix[12], openGlMatrix[13], openGlMatrix[14]};
		// double targetPosition[] = {0., 0., 0.};
		gluLookAt(
		    /*Position=*/position[0], position[1], position[2],
		    /*Look at (world space)=*/targetPosition[0], targetPosition[1], targetPosition[2],
		    /*Up vector=*/0, 1, 0);

		// glTranslatef(0.f, cameraHeight, cameraPullback);
	}

	// Manual method
	if (true)
	{
		glRotatef(180.f, 0, 1, 0);
		// glTranslatef(0.f, -cameraHeight, cameraPullback);
		glTranslatef(0.f, -4.f, 8.f);
		// double rotMatNoTransform[][4] = {
		//     {openGlMatrix[0], openGlMatrix[1], openGlMatrix[2], 0},
		//     {openGlMatrix[4], openGlMatrix[5], openGlMatrix[6], 0},
		//     {openGlMatrix[7], openGlMatrix[8], openGlMatrix[9], 0},
		//     {0., 0., 0., 1.},
		// };

		glMultMatrixd(openGlMatrix);
		// glMultMatrixd(rotMatNoTransform);
		// glRotatef(camRot[0], 1, 0, 0);
		// glRotatef(camRot[2], 0, 0, 1);

		// Pitch camera
		// glRotated(cameraPitch, 1, 0, 0);
	}

	glPushMatrix();
}

void Camera::UpdateEnd()
{
	// Reset translation vector
	camTranslate[0] = 0;
	camTranslate[1] = 0;
	camTranslate[2] = 0;

	glPopMatrix();
}
