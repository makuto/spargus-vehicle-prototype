#include "Camera.hpp"

#include <cmath>
#include <GL/glu.h>
#include <SFML/OpenGL.hpp>
#include "graphics/graphics.hpp"
#include "input/input.hpp"

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

#include "Render_Horde3D.hpp"
#include <Horde3D.h>

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

	hmm_vec3 rotateYAxis = {0, 1, 0};
	hmm_vec3 rotateXAxis = {1, 0, 0};
	hmm_mat4 camMatrix = HMM_Rotate(camRot[0], rotateXAxis);
	hmm_mat4 camMatrixRotY = HMM_Rotate(camRot[1], rotateYAxis);
	camMatrix = HMM_MultiplyMat4(camMatrix, camMatrixRotY);	
	camMatrix.Elements[3][0] = camPos[0];
	camMatrix.Elements[3][1] = camPos[1];
	camMatrix.Elements[3][2] = camPos[2];
	h3dSetNodeTransMat(hordeCamera, reinterpret_cast<float*>(&camMatrix.Elements[0][0]));
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
	// glLoadIdentity();
	// gluLookAt(0, 0, 1, 0, 0, -1, 0, 1, 0);
	// glRotatef(camRot[0], 1, 0, 0);
	// glRotatef(camRot[1], 0, 1, 0);
	// glRotatef(camRot[2], 0, 0, 1);

	// glTranslatef(-camPos[0], camPos[1], -camPos[2]);
	// glPushMatrix();
}

void Camera::ChaseCamera(double* openGlMatrix)
{
	const double cameraHeight = 4.0;
	const double cameraPullback = 15.0;
	// const double cameraPitch = 30.0;
	
	hmm_mat4* carMatrix = reinterpret_cast<hmm_mat4*>(openGlMatrix);

	hmm_vec3 rotateYAxis = {0, 1, 0};
	hmm_mat4 camMatrix = HMM_Rotate(180.f, rotateYAxis);
	camMatrix.Elements[3][0] = 0.f;
    camMatrix.Elements[3][1] = -cameraHeight;
    camMatrix.Elements[3][2] = cameraPullback;

	camMatrix = HMM_MultiplyMat4(camMatrix, *carMatrix);

	// Pitch camera
	// glRotated(cameraPitch, 1, 0, 0);

	// Not sure if this cast is necessary (just use .Elements?)
	h3dSetNodeTransMat(hordeCamera, reinterpret_cast<float*>(camMatrix.Elements));
}

void Camera::UpdateEnd()
{
	// Reset translation vector
	camTranslate[0] = 0;
	camTranslate[1] = 0;
	camTranslate[2] = 0;

	// glPopMatrix();
}
