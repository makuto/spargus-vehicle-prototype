#include "Horde3DCore.hpp"

#include "GraphicsInterface.hpp"

// From http://www.horde3d.org/docs/html/_tutorial.html
#include <Horde3D.h>
#include <Horde3DUtils.h>

#include "Math.hpp"
#include "Utilities.hpp"

H3DNode model = 0;
H3DNode hordeCamera = 0;
H3DNode buggyNode = 0;
H3DNode buggyWheelNodes[4];
H3DRes buggyWheelRes = 0;
H3DRes fontMaterialRes = 0;
H3DRes pipeRes = 0;

const float cameraFov = 60.f;
const float cameraNearPlane = 0.5f;
const float cameraFarPlane = 4096.0f;

bool g_graphicsIntialized = false;

// Note that the name must be unique (because Horde3D uses strings as references)
H3DNode CreateProceduralGeometry(const char* geoName, float* vertices, unsigned int* indices,
                                 // Optional
                                 short* normals, short* tangents, short* bitangents,
                                 float* texture1UVs, float* texture2UVs,
                                 // Required
                                 int numVertices, int numIndices)
{
	// This copies the geometry data
	H3DRes proceduralCubeGeo =
	    h3dutCreateGeometryRes(geoName, numVertices, numIndices, vertices, indices, normals,
	                           tangents, bitangents, texture1UVs, texture2UVs);

	if (proceduralCubeGeo)
	{
		H3DNode model = h3dAddModelNode(H3DRootNode, geoName, proceduralCubeGeo);
		// Material resource used by Mesh node
		H3DRes materialRes =
		    h3dAddResource(H3DResTypes::Material, "assets/World_Material.001.material.xml", 0);
		h3dutLoadResourcesFromDisk("Content");

		// first triangle index of mesh in Geometry resource of parent Model node
		int batchStart = 0;
		// number of triangle indices used for drawing mesh
		int batchCount = numIndices;
		// first vertex in Geometry resource of parent Model node
		int vertRStart = 0;
		// last vertex in Geometry resource of parent Model node
		int vertREnd = numVertices - 1;

		H3DNode mesh = h3dAddMeshNode(model, geoName, materialRes, batchStart, batchCount,
		                              vertRStart, vertREnd);

		return mesh;

		// H3DNode mesh = h3dAddMeshNode(model, "ProceduralCube_Mesh", 0, 0, 6, 0, 3);

		// H3DNode cube = h3dAddNodes(H3DRootNode, proceduralCube);
		// h3dSetNodeTransform(cube, 0, 0, 0, 0, 0, 0, 0, 10, 0);
	}

	return 0;
}

namespace Graphics
{
static void TestProceduralGeometry_Cube()
{
	float size = 10.f;
	float vertices[] = {
	    // Top fl [0]
	    -1.f * size,
	    1.f * size,
	    -1.f * size,
	    // Top fr [1]
	    1.f * size,
	    1.f * size,
	    -1.f * size,
	    // Top bl [2]
	    -1.f * size,
	    1.f * size,
	    1.f * size,
	    // Top br [3]
	    1.f * size,
	    1.f * size,
	    1.f * size,
	    // Bottom fl [4]
	    -1.f * size,
	    -1.f * size,
	    -1.f * size,
	    // Bottom fr [5]
	    1.f * size,
	    -1.f * size,
	    -1.f * size,
	    // Bottom bl [6]
	    -1.f * size,
	    -1.f * size,
	    1.f * size,
	    // Bottom br [7]
	    1.f * size,
	    -1.f * size,
	    1.f * size,
	};

	unsigned int numVertices = ArraySize(vertices) / 3;

	unsigned int indices[] = {// Top
	                          0, 2, 1, 1, 2, 3,
	                          // Front
	                          0, 1, 4, 4, 1, 5,
	                          // Left
	                          6, 2, 4, 4, 2, 0,
	                          // Back
	                          3, 2, 6, 6, 7, 3,
	                          // Right
	                          5, 1, 7, 7, 1, 3,
	                          // Bottom
	                          6, 4, 7, 7, 4, 5};

	unsigned int numIndices = ArraySize(indices);

	CreateProceduralGeometry("ProceduralGeo_Cube", vertices, indices, nullptr, nullptr, nullptr,
	                         nullptr, nullptr, numVertices, numIndices);
}

void Initialize(int winWidth, int winHeight)
{
	// Initialize engine
	h3dInit(H3DRenderDevice::OpenGL4);

	// Environment
	// H3DRes envRes = h3dAddResource(H3DResTypes::SceneGraph, "assets/World.scene.xml", 0);

	// Skybox
	H3DRes skyBoxRes = h3dAddResource(H3DResTypes::SceneGraph, "models/skybox/skybox.scene.xml", 0);

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

	// TestProceduralGeometry_Cube();

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

	// Add skybox
	H3DNode sky = h3dAddNodes(H3DRootNode, skyBoxRes);
	h3dSetNodeTransform(sky, 0, 0, 0, 0, 0, 0, 2100, 500, 2100);
	h3dSetNodeFlags(sky, H3DNodeFlags::NoCastShadow, true);

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

void SetViewport(int x, int y, int width, int height)
{
	if (!hordeCamera || !pipeRes)
		return;

	// Resize viewport
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportXI, x);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportYI, y);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportWidthI, width);
	h3dSetNodeParamI(hordeCamera, H3DCamera::ViewportHeightI, height);

	// Set virtual camera parameters
	float aspectRatio = (float)width / height;
	h3dSetupCameraView(hordeCamera, cameraFov, aspectRatio, cameraNearPlane,
	                   cameraFarPlane);
	h3dResizePipelineBuffers(pipeRes, width, height);
}

void OnWindowResized(int winWidth, int winHeight)
{
	SetViewport(0, 0, winWidth, winHeight);
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
