// From http://www.horde3d.org/docs/html/_tutorial.html
#include <Horde3D.h>
#include <Horde3DUtils.h>

H3DNode model = 0, cam = 0;

void hordeInitialize(int winWidth, int winHeight)
{
	// Initialize engine
	h3dInit(H3DRenderDevice::OpenGL4);

	// Add pipeline resource
	H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline, "pipelines/forward.pipeline.xml", 0);
	// Add model resource
	H3DRes modelRes = h3dAddResource(H3DResTypes::SceneGraph, "models/man/man.scene.xml", 0);
	// Add animation resource
	H3DRes animRes = h3dAddResource(H3DResTypes::Animation, "animations/man.anim", 0);
	// Load added resources
	h3dutLoadResourcesFromDisk("Horde3D/Horde3D/Binaries/Content");

	// Add model to scene
	model = h3dAddNodes(H3DRootNode, modelRes);
	// Apply animation
	h3dSetupModelAnimStage(model, 0, animRes, 0, "", false);

	// Add light source
	H3DNode light = h3dAddLightNode(H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP");
	// Set light position and radius
	h3dSetNodeTransform(light, 0, 20, 0, 0, 0, 0, 1, 1, 1);
	h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 50.0f);

	// Add camera
	cam = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);

	// Setup viewport and render target sizes
	h3dSetNodeParamI(cam, H3DCamera::ViewportXI, 0);
	h3dSetNodeParamI(cam, H3DCamera::ViewportYI, 0);
	h3dSetNodeParamI(cam, H3DCamera::ViewportWidthI, winWidth);
	h3dSetNodeParamI(cam, H3DCamera::ViewportHeightI, winHeight);
	h3dSetupCameraView(cam, 45.0f, (float)winWidth / winHeight, 0.5f, 2048.0f);
	h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);
}

void hordeUpdate(float fps)
{
	static float t = 0;

	// Increase animation time
	t = t + 10.0f * (1 / fps);

	// Play animation
	h3dSetModelAnimParams(model, 0, t, 1.0f);
	h3dUpdateModel(model, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);

	// Set new model position
	h3dSetNodeTransform(model, t * 10, 0, 0,  // Translation
	                    0, 0, 0,              // Rotation
	                    1, 1, 1);             // Scale

	// Render scene
	h3dRender(cam);

	// Finish rendering of frame
	h3dFinalizeFrame();
}

void hordeRelease()
{
	// Release engine
	h3dRelease();
}
