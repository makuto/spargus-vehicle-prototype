#include "GraphicsNode_Horde3D.hpp"
#include "GraphicsNode.hpp"

#include "Math.hpp"

#include <Horde3D.h>
#include <Horde3DUtils.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace Graphics
{
void Node::Initialize(const char* requestedResource)
{
	// TODO: Remove allocation
	resource = new NodeResourceReference();

	char resourceNameBuffer[1024];
	snprintf(resourceNameBuffer, sizeof(resourceNameBuffer), "assets/%s.scene.xml",
	         requestedResource);

	resource->resource = h3dAddResource(H3DResTypes::SceneGraph, resourceNameBuffer, 0);
	resource->node = h3dAddNodes(H3DRootNode, resource->resource);
}

void Node::TransformUpdated()
{
	h3dSetNodeTransMat(resource->node, glmMatrixToHordeMatrixRef(transform));
}

}  // namespace Graphics
