#include "Camera.hpp"

#include <cmath>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#include <glm/mat4x4.hpp>  // mat4
#include <glm/vec3.hpp>    // vec3
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>  //radians
#include <glm/ext/matrix_transform.hpp>

#include "GraphicsInterface.hpp"
#include "Logging.hpp"
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
	float freeCamHorizontalSpeed = 30.f;
	float freeCamVerticalSpeed = 30.f;

	// Slow camera motion while holding shift
	if (input.isPressed(inputCode::RShift) || input.isPressed(inputCode::LShift))
	{
		freeCamHorizontalSpeed = 2.f;
		freeCamVerticalSpeed = 2.f;
	}

	if (input.isPressed(inputCode::W))
	{
		/* yrotrad = (yrot / 180 * 3.141592654f);
		    xrotrad = (xrot / 180 * 3.141592654f);
		    xpos += float(sin(yrotrad)) ;
		    zpos -= float(cos(yrotrad)) ;
		 * */
		camTranslate[0] +=
		    (freeCamHorizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -=
		    (freeCamHorizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::S))
	{
		camTranslate[0] -=
		    (freeCamHorizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
		camTranslate[2] +=
		    (freeCamHorizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		// camTranslate[2]=50*win.GetFrameTime();
		// camPos[1]=-10;
	}
	if (input.isPressed(inputCode::A))
	{
		camTranslate[0] -=
		    (freeCamHorizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] -=
		    (freeCamHorizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
	}
	if (input.isPressed(inputCode::D))
	{
		camTranslate[0] +=
		    (freeCamHorizontalSpeed * frameTime) * float(cos(camRot[1] / 180 * 3.141592654));
		camTranslate[2] +=
		    (freeCamHorizontalSpeed * frameTime) * float(sin(camRot[1] / 180 * 3.141592654));
	}

	if (input.isPressed(inputCode::Q))
	{
		camTranslate[1] -= freeCamVerticalSpeed * frameTime;
	}
	if (input.isPressed(inputCode::E))
	{
		camTranslate[1] += freeCamVerticalSpeed * frameTime;
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

	glm::mat4 camMatrix(1.f);
	camMatrix = glm::translate(glm::mat4(1.f), -camPos);
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.f), glm::radians(camRot[1]), UpAxis);
	glm::rotate(rotationMatrix, glm::radians(camRot[0]), LeftAxis);
	glm::rotate(rotationMatrix, glm::radians(camRot[2]), ForwardAxis);
	camMatrix = rotationMatrix * camMatrix;
	// TODO Move this to graphics internals?
	camMatrix = glm::inverse(camMatrix);
	Graphics::SetCameraTransform(camMatrix);
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

// Similar to chase camera, only no angular constraints
void Camera::OrbitCamera(const glm::mat4& targetMatrix)
{
	const double cameraHeight = 4.0;
	const float pullbackRadius = 15.f;

	glm::vec3 targetTranslateVec(targetMatrix[3]);

	glm::vec3 translateCameraVec = {0.f, cameraHeight, 0.f};
	glm::vec3 camPos =
	    translateCameraVec + targetTranslateVec + (targetCameraDirection * pullbackRadius);
	glm::vec3 targetPos = targetTranslateVec;
	glm::vec3 upVector = {0.f, 1.f, 0.f};
	glm::mat4 camMatrix(1.f);
	camMatrix = glm::lookAt(camPos, targetPos, upVector);

	camMatrix = glm::inverse(camMatrix);

	Graphics::SetCameraTransform(camMatrix);
}

void Camera::ChaseCamera(const glm::mat4& targetMatrix)
{
	const double cameraHeight = 4.0;
	const float pullbackRadius = 15.f;
	
	glm::vec3 targetTranslateVec(targetMatrix[3]);

	glm::vec3 translateCameraVec = {0.f, cameraHeight, 0.f};
	glm::vec3 camPos =  translateCameraVec + targetTranslateVec + (targetCameraDirection * pullbackRadius);
	glm::vec3 targetPos = targetTranslateVec;
	glm::vec3 upVector = {0.f, 1.f, 0.f};
	glm::mat4 camMatrix(1.f);
	camMatrix = glm::lookAt(camPos, targetPos, upVector);

	camMatrix = glm::inverse(camMatrix);

	Graphics::SetCameraTransform(camMatrix);
}

void Camera::UpdateEnd()
{
	// Reset translation vector
	camTranslate[0] = 0;
	camTranslate[1] = 0;
	camTranslate[2] = 0;

	// glPopMatrix();
}
