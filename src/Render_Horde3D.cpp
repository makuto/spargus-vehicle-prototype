// From http://www.horde3d.org/docs/html/_tutorial.html
#include <Horde3D.h>
#include <Horde3DUtils.h>

#include <iostream>

H3DNode model = 0;
H3DNode hordeCamera = 0;
H3DRes fontMaterialRes = 0;

void hordeInitialize(int winWidth, int winHeight)
{
	// Initialize engine
	h3dInit(H3DRenderDevice::OpenGL4);
	// h3dInit(H3DRenderDevice::OpenGL2);

	// Environment
	H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "models/sphere/sphere.scene.xml", 0);

	// Add pipeline resource
	H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/hdr.pipeline.xml", 0);
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
	h3dutLoadResourcesFromDisk("Horde3D/Horde3D/Binaries/Content");

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
	h3dSetupCameraView(hordeCamera, 60.0f, (float)winWidth / winHeight, 0.5f, 2048.0f);
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

void hordeUpdate(float fps)
{
	static float t = 0;

	// Increase animation time
	t += fps;

	std::cout << t << "\n";
	// Play animation
	h3dSetModelAnimParams(model, 0, t * 24.f, 1.0f);
	h3dUpdateModel(model, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);

	// // Set new model position
	// h3dSetNodeTransform(model, t * 10, 0, 0,  // Translation
	//                     0, 0, 0,              // Rotation
	//                     1, 1, 1);             // Scale
	
	h3dutShowText("Test", /*x, y*/10, 10, /*size*/10, /*r,g,b*/1.f, 0.f, 0.f, fontMaterialRes);

	// Render scene
	h3dRender(hordeCamera);

	// Finish rendering of frame
	h3dFinalizeFrame();
}

void hordeRelease()
{
	// Release engine
	h3dRelease();
}
