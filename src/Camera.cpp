#include "Camera.hpp"

#include <cmath>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>    // vec3
#include <glm/trigonometric.hpp>  //radians
#include <glm/ext/matrix_transform.hpp>

#include "Render_Horde3D.hpp"
#include <Horde3D.h>

#include "Math.hpp"

#include <iostream>

Camera::Camera(window& winOwner) : win(winOwner)
{
	winBase = win.getBase();
	win.getBase()->setMouseCursorVisible(false);
	sf::Mouse::setPosition(sf::Vector2i(win.getWidth() / 2, win.getHeight() / 2), *winBase);

	prevX = winBase->getSize().x / 2;
	prevY = win.getHeight() / 2;
}

void Camera::FreeCam(inputManager& input, float frameTime)
{
	float horizontalSpeed = 20.f;
	float verticalSpeed = 20.f;

	// Slow camera motion while holding shift
	if (input.isPressed(inputCode::RShift) || input.isPressed(inputCode::LShift))
	{
		horizontalSpeed = 2.f;
		verticalSpeed = 2.f;
	}

	if (input.isPressed(inputCode::W))
	{
		/* yrotrad = (yrot / 180 * 3.141592654f);
		    xrotrad = (xrot / 180 * 3.141592654f);
		    xpos += float(sin(yrotrad)) ;
		    zpos -= float(cos(yrotrad)) ;
		 * */
		camTranslate[0] +=
		    (horizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -=
		    (horizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::S))
	{
		camTranslate[0] -=
		    (horizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] +=
		    (horizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		// camTranslate[2]=50*win.GetFrameTime();
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::A))
	{
		camTranslate[0] -=
		    (horizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -=
		    (horizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
	}
	if (input.isPressed(inputCode::D))
	{
		camTranslate[0] +=
		    (horizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] +=
		    (horizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
	}

	if (input.isPressed(inputCode::Q))
	{
		camTranslate[1] -= verticalSpeed * frameTime;
	}
	if (input.isPressed(inputCode::E))
	{
		camTranslate[1] += verticalSpeed * frameTime;
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

	h3dSetNodeTransform(hordeCamera, camPos[0], camPos[1], camPos[2], -camRot[0], -camRot[1], 0.f,
	                    1.f, 1.f, 1.f);
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
}

void Camera::ChaseCamera(double* openGlMatrix)
{
	const double cameraHeight = 4.0;
	const double cameraPullback = 15.0;
	// const double cameraPitch = 30.0;

	// Copy opengl matrix into something we can manipulate easily
	glm::mat4 carMatrix;
	openGlMatrixToGlmMat4(openGlMatrix, carMatrix);
	
	// Invert direction for camera
	glm::vec3 rotateYAxis = {0.f, 1.f, 0.f};
	glm::mat4 camMatrix(1.f);
	
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(180.f), rotateYAxis);
	// Pull camera up and back
	glm::vec3 translateVec = {0.f, -cameraHeight, -cameraPullback};
	camMatrix = glm::translate(camMatrix, translateVec);

	camMatrix = camMatrix * rotationMatrix * carMatrix;

	// Pitch camera
	// glRotated(cameraPitch, 1, 0, 0);

	camMatrix = glm::inverse(camMatrix);
	
	// Not sure if this cast is necessary (just use .Elements?)
	h3dSetNodeTransMat(hordeCamera, &camMatrix[0][0]);
}

void Camera::UpdateEnd()
{
	// Reset translation vector
	camTranslate[0] = 0;
	camTranslate[1] = 0;
	camTranslate[2] = 0;

	// glPopMatrix();
}
