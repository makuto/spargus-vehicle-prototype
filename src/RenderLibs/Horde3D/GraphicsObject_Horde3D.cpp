#include "GraphicsObject_Horde3D.hpp"
#include "GraphicsObject.hpp"

#include "Horde3DCore.hpp"
#include "Logging.hpp"
#include "Math.hpp"

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
	h3dutLoadResourcesFromDisk("Content");

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
                                int numTriangles, int numIndices)
{
	// If you're getting this, make sure to call Graphics::Initialize() before any nodes initialize
	assert(g_graphicsIntialized);

	// TODO: Remove allocation
	resource = new ResourceReference();

	resource->node = TestProceduralGeometry(geoName, vertices, indices, numTriangles, numIndices);

	if (!resource->node)
	{
		LOGE << "Could not create Horde3D Mesh Node for '" << geoName << "' (requested a "
		     << numTriangles << " mesh)";
		return;
	}

	// Set the transform in case this node has an updated transform already
	TransformUpdated();
}
}  // namespace Graphics
