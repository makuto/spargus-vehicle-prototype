#include "GraphicsInterface.hpp"

// From http://www.horde3d.org/docs/html/_tutorial.html
#include <Horde3D.h>
#include <Horde3DUtils.h>

#include "Math.hpp"

H3DNode model = 0;
H3DNode hordeCamera = 0;
H3DNode buggyNode = 0;
H3DNode buggyWheelNodes[4];
H3DRes buggyWheelRes = 0;
H3DRes fontMaterialRes = 0;
H3DRes pipeRes = 0;

const float cameraFov = 60.f;
const float cameraNearPlane = 0.5f;
const float cameraFarPlane = 2048.0f;

bool g_graphicsIntialized = false;

namespace Graphics
{
void Initialize(int winWidth, int winHeight)
{
	// Initialize engine
	h3dInit(H3DRenderDevice::OpenGL4);

	// Environment
	H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "assets/World.scene.xml", 0);
	// H3DRes buggyRes = h3dAddResource(H3DResTypes::SceneGraph, "assets/BasicBuggy.scene.xml", 0);
	H3DRes buggyRes =
	    h3dAddResource(H3DResTypes::SceneGraph, "assets/BasicBuggy_Chassis.scene.xml", 0);
	buggyWheelRes = h3dAddResource(H3DResTypes::SceneGraph, "assets/Wheel_Rear.scene.xml", 0);

	// Scale reference
	if (false)
	{
		H3DRes scaleReferenceRes =
		    h3dAddResource(H3DResTypes::SceneGraph, "assets/ScaleReference.scene.xml", 0);

		H3DNode scaleReferenceNode = h3dAddNodes(H3DRootNode, scaleReferenceRes);
		h3dSetNodeTransform(scaleReferenceNode, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f);
	}

	// H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0);
	// Add model resource
	// H3DRes modelRes = h3dAddResource(H3DResTypes::SceneGraph, "models/knight/knight.scene.xml",
	// 0);
	// // H3DRes modelRes = h3dAddResource(H3DResTypes::SceneGraph, "models/man/man.scene.xml", 0);
	// // Add animation resource
	// // H3DRes animRes = h3dAddResource(H3DResTypes::Animation, "animations/man.anim", 0);
	// H3DRes animRes = h3dAddResource(H3DResTypes::Animation, "animations/knight_order.anim", 0);

	// Add pipeline resource
	pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0);
	// Add font
	fontMaterialRes = h3dAddResource(H3DResTypes::Material, "overlays/font.material.xml", 0);
	// Load added resources
	h3dutLoadResourcesFromDisk("Content");

	// Add environment
	// H3DNode env = h3dAddNodes(H3DRootNode, envRes);
	// h3dSetNodeTransform(env, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f);

	// Apply animation
	// h3dSetupModelAnimStage(model, 0, animRes, 0, "", false);

	// Add headlight source
	// TODO Figure out graphics layer for this
	if (false)
	{
		H3DNode light = h3dAddLightNode(buggyNode, "Headlight", 0, "LIGHTING", "SHADOWMAP");
		// Set light position and radius
		h3dSetNodeTransform(light, /*transform=*/0, 0, 5.f, /*rotation=*/0, 180.f, 0.f,
		                    /*scale=*/1, 1, 1);
		h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 50.0f);
		H3DNode matRes = h3dFindResource(H3DResTypes::Material, "pipelines/postHDR.material.xml");
		h3dSetMaterialUniform(matRes, "hdrExposure", 2.5f, 0, 0, 0);
	}

	// Add camera
	hordeCamera = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);

	// Setup viewport and render target sizes
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportWidthI, winWidth);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportHeightI, winHeight);
	h3dSetupCameraView(hordeCamera, 60.0f, (float)winWidth / winHeight, cameraNearPlane,
	                   cameraFarPlane);
	h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);

	// h3dSetOption(H3DOptions::DebugViewMode, 1.0f);
	// h3dSetOption(H3DOptions::MaxLogLevel, 10000.f);
	// h3dSetOption(H3DOptions::WireframeMode, 1.0f);

	// From knight
	{
		// Shader for deferred shading
		H3DRes lightMatRes =
		    h3dAddResource(H3DResTypes::Material, "materials/light.material.xml", 0);
		// Add light source
		H3DNode light =
		    h3dAddLightNode(H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP");
		h3dSetNodeTransform(light, 0, 15, 10, -60, 0, 0, 1, 1, 1);
		h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 30);
		h3dSetNodeParamF(light, H3DLight::FovF, 0, 90);
		h3dSetNodeParamI(light, H3DLight::ShadowMapCountI, 1);
		h3dSetNodeParamF(light, H3DLight::ShadowMapBiasF, 0, 0.01f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 0, 1.0f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 1, 0.8f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 2, 0.7f);
		h3dSetNodeParamF(light, H3DLight::ColorMultiplierF, 0, 1.0f);

		// Customize post processing effects
		H3DNode matRes = h3dFindResource(H3DResTypes::Material, "pipelines/postHDR.material.xml");
		h3dSetMaterialUniform(matRes, "hdrExposure", 2.5f, 0, 0, 0);
		h3dSetMaterialUniform(matRes, "hdrBrightThres", 0.5f, 0, 0, 0);
		h3dSetMaterialUniform(matRes, "hdrBrightOffset", 0.08f, 0, 0, 0);
	}

	g_graphicsIntialized = true;
}

static void hordeTestInitialize(int winWidth, int winHeight)
{
	// Initialize engine
	h3dInit(H3DRenderDevice::OpenGL4);
	// h3dInit(H3DRenderDevice::OpenGL2);

	// Environment
	// H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "models/sphere/sphere.scene.xml", 0);
	H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "assets/BasicBuggy.scene.xml", 0);

	// Add pipeline resource
	pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0);
	// H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0);
	// Add model resource
	H3DRes modelRes = h3dAddResource(H3DResTypes::SceneGraph, "models/knight/knight.scene.xml", 0);
	// H3DRes modelRes = h3dAddResource(H3DResTypes::SceneGraph, "models/man/man.scene.xml", 0);
	// Add animation resource
	// H3DRes animRes = h3dAddResource(H3DResTypes::Animation, "animations/man.anim", 0);
	H3DRes animRes = h3dAddResource(H3DResTypes::Animation, "animations/knight_order.anim", 0);
	// Add font
	fontMaterialRes = h3dAddResource(H3DResTypes::Material, "overlays/font.material.xml", 0);
	// Load added resources
	h3dutLoadResourcesFromDisk("Content");

	// Add environment
	H3DNode env = h3dAddNodes(H3DRootNode, envRes);
	h3dSetNodeTransform(env, 0, -20, 0, 0, 0, 0, 20, 20, 20);

	H3DNode model2 = h3dAddNodes(H3DRootNode, modelRes);
	h3dSetNodeTransform(model2, -10, -5, 0, 0, 0, 0, 0.5f, 0.5f, 0.5f);

	// Add model to scene
	model = h3dAddNodes(H3DRootNode, modelRes);
	h3dSetNodeTransform(model, 0, -3, 0, 0, 0, 0, 0.5f, 0.5f, 0.5f);
	// Apply animation
	h3dSetupModelAnimStage(model, 0, animRes, 0, "", false);

	// Add light source
	H3DNode light = h3dAddLightNode(H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP");
	// Set light position and radius
	h3dSetNodeTransform(light, 0, 20, 0, 0, 0, 0, 1, 1, 1);
	h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 500.0f);

	// Add camera
	hordeCamera = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);

	// Setup viewport and render target sizes
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportWidthI, winWidth);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportHeightI, winHeight);
	h3dSetupCameraView(hordeCamera, cameraFov, (float)winWidth / winHeight, 0.5f, 2048.0f);
	h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);

	// h3dSetOption(H3DOptions::DebugViewMode, 1.0f);
	h3dSetOption(H3DOptions::MaxLogLevel, 10000.f);
	// h3dSetOption(H3DOptions::WireframeMode, 1.0f);

	// From knight
	{
		// Shader for deferred shading
		H3DRes lightMatRes =
		    h3dAddResource(H3DResTypes::Material, "materials/light.material.xml", 0);
		// Add light source
		H3DNode light =
		    h3dAddLightNode(H3DRootNode, "Light1", lightMatRes, "LIGHTING", "SHADOWMAP");
		h3dSetNodeTransform(light, 0, 15, 10, -60, 0, 0, 1, 1, 1);
		h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 30);
		h3dSetNodeParamF(light, H3DLight::FovF, 0, 90);
		h3dSetNodeParamI(light, H3DLight::ShadowMapCountI, 1);
		h3dSetNodeParamF(light, H3DLight::ShadowMapBiasF, 0, 0.01f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 0, 1.0f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 1, 0.8f);
		h3dSetNodeParamF(light, H3DLight::ColorF3, 2, 0.7f);
		h3dSetNodeParamF(light, H3DLight::ColorMultiplierF, 0, 1.0f);

		// Customize post processing effects
		H3DNode matRes = h3dFindResource(H3DResTypes::Material, "pipelines/postHDR.material.xml");
		h3dSetMaterialUniform(matRes, "hdrExposure", 2.5f, 0, 0, 0);
		h3dSetMaterialUniform(matRes, "hdrBrightThres", 0.5f, 0, 0, 0);
		h3dSetMaterialUniform(matRes, "hdrBrightOffset", 0.08f, 0, 0, 0);
	}
}

void OnWindowResized(int winWidth, int winHeight)
{
	if (!hordeCamera || !pipeRes)
		return;

	// Resize viewport
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportWidthI, winWidth);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportHeightI, winHeight);

	// Set virtual camera parameters
	h3dSetupCameraView(hordeCamera, cameraFov, (float)winWidth / winHeight, cameraNearPlane,
	                   cameraFarPlane);
	// TODO: Fix buffer resize
	h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);
}

void Update(float fps)
{
	if (model)
	{
		static float t = 0;

		// Increase animation time
		t += fps;

		// Play animation
		h3dSetModelAnimParams(model, 0, t * 24.f, 1.0f);
		h3dUpdateModel(model, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);

		// // Set new model position
		// h3dSetNodeTransform(model, t * 10, 0, 0,  // Translation
		//                     0, 0, 0,              // Rotation
		//                     1, 1, 1);             // Scale
	}

	h3dutShowText("Test", /*x, y*/ 10, 10, /*size*/ 10, /*r,g,b*/ 1.f, 0.f, 0.f, fontMaterialRes);

	// Render scene
	h3dRender(hordeCamera);

	// Finish rendering of frame
	h3dFinalizeFrame();
}

void Destroy()
{
	// Release engine
	h3dRelease();
}

void SetCameraTransform(const glm::mat4& newTransform)
{
	h3dSetNodeTransMat(hordeCamera, glmMatrixToHordeMatrixRef(newTransform));
}

glm::mat4 GetCameraTransformCopy()
{
	glm::mat4 cameraMat(1.f);
	const float* cameraTranslationMat = 0;
	h3dGetNodeTransMats(hordeCamera, 0, &cameraTranslationMat);

	// In case of an invalid camera (e.g. pipeline not set) return identity matrix
	if (cameraTranslationMat)
	{
		openGlMatrixToGlmMat4(cameraTranslationMat, cameraMat);
	}

	return cameraMat;
}

glm::mat4 GetCameraProjectionMatrixCopy()
{
	glm::mat4 projectionMat(1.f);
	if (hordeCamera)
	{
		float hordeProjectionMat[16];
		h3dGetCameraProjMat(hordeCamera, hordeProjectionMat);
		openGlMatrixToGlmMat4(hordeProjectionMat, projectionMat);
	}
	return projectionMat;
}
}  // namespace Graphics
