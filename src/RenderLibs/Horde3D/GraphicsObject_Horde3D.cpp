#include "GraphicsObject_Horde3D.hpp"
#include "GraphicsObject.hpp"

#include "Horde3DCore.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Performance.hpp"

#include <Horde3D.h>
#include <Horde3DUtils.h>

#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Graphics
{
// TODO: Prevent reinitialization
void Object::Initialize(const char* requestedResource)
{
	PerfTimeNamedScope(HordeRenderScope, "Graphics Object Init", tracy::Color::Chocolate4);
	PerfSetNameFormat(HordeRenderScope, "Graphics Object Init '%s'", requestedResource);

	// If you're getting this, make sure to call Graphics::Initialize() before any nodes initialize
	assert(g_graphicsIntialized);

	// TODO: Remove allocation
	resource = new ResourceReference();

	char resourceNameBuffer[1024];
	snprintf(resourceNameBuffer, sizeof(resourceNameBuffer), "assets/%s.scene.xml",
	         requestedResource);

	// Request and load resource
	// TODO: Leaking resources?
	resource->resource = h3dAddResource(H3DResTypes::SceneGraph, resourceNameBuffer, 0);
	if (!resource->resource)
	{
		LOGE << "Resource " << resourceNameBuffer << " not found";
		return;
	}
	// Hitting an assert somewhere in here? Make sure the OpenGL SFML context is active
	// See https://www.sfml-dev.org/tutorials/2.5/window-opengl.php
	{
		PerfTimeNamedScope(HordeRenderScope, "Graphics Object Load Resources", tracy::Color::Red1);
		HordeLoadResources();
	}

	// TODO: Remove node on destroy
	resource->node = h3dAddNodes(H3DRootNode, resource->resource);
	if (!resource->node)
	{
		LOGE << "Could not create Horde3D Node for Resource " << resourceNameBuffer << " (resource "
		     << resource->resource << ")";
		return;
	}

	// Set the transform in case this node has an updated transform already
	TransformUpdated();
}

void Object::Destroy()
{
	if (resource && resource->node)
		h3dRemoveNode(resource->node);
}

void Object::TransformUpdated()
{
	if (resource && resource->node)
		h3dSetNodeTransMat(resource->node, glmMatrixToHordeMatrixRef(transform));
}

void ProceduralMesh::Initialize(const char* geoName, float* vertices, unsigned int* indices,
                                // Optional
                                short* normals, short* tangents, short* bitangents,
                                float* texture1UVs, float* texture2UVs,
                                // Required
                                int numVertices, int numIndices)
{
	PerfTimeNamedScope(HordeRenderScope, "Horde Init Procedural Geometry",
	                   tracy::Color::LightGreen);

	// If you're getting this, make sure to call Graphics::Initialize() before any nodes initialize
	assert(g_graphicsIntialized);

	// TODO: Remove allocation
	resource = new ResourceReference();

	resource->node =
	    CreateProceduralGeometry(geoName, vertices, indices, normals, tangents, bitangents,
	                             texture1UVs, texture2UVs, numVertices, numIndices);

	if (!resource->node)
	{
		LOGE << "Could not create Horde3D Mesh Node for '" << geoName << "' (requested a "
		     << numVertices << " vertex mesh)";
		return;
	}

	// Set the transform in case this node has an updated transform already
	TransformUpdated();
}

void Light::Initialize(const char* lightName)
{
	PerfTimeNamedScope(HordeRenderScope, "Graphics Light Init", tracy::Color::Chocolate4);
	PerfSetNameFormat(HordeRenderScope, "Graphics Light Init '%s'", lightName);

	// If you're getting this, make sure to call Graphics::Initialize() before any nodes initialize
	assert(g_graphicsIntialized);

	// TODO: Remove allocation
	object.resource = new ResourceReference();

	object.resource->node = h3dAddLightNode(H3DRootNode, lightName, 0, "LIGHTING", "SHADOWMAP");
	h3dSetNodeParamF(object.resource->node, H3DLight::RadiusF, 0, 50.0f);
	h3dSetNodeParamF(object.resource->node, H3DLight::FovF, 0, 90);
	h3dSetNodeParamI(object.resource->node, H3DLight::ShadowMapCountI, 1);
	h3dSetNodeParamF(object.resource->node, H3DLight::ShadowMapBiasF, 0, 0.01f);
	h3dSetNodeParamF(object.resource->node, H3DLight::ColorF3, 0, 1.0f);
	h3dSetNodeParamF(object.resource->node, H3DLight::ColorF3, 1, 0.8f);
	h3dSetNodeParamF(object.resource->node, H3DLight::ColorF3, 2, 0.7f);
	h3dSetNodeParamF(object.resource->node, H3DLight::ColorMultiplierF, 0, 2.0f);

	// Set the transform in case this node has an updated transform already
	object.TransformUpdated();
}
}  // namespace Graphics
