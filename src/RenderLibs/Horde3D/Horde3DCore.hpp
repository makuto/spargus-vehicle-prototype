#pragma once

#include <Horde3D.h>

extern bool g_graphicsIntialized;

// (This comment copied from Horde3DUtils.h)
// 		Creates a Geometry resource from specified vertex data.
	
// 	Details:
// 		This utility function allocates and initializes a Geometry resource
// 		with the specified vertex attributes and indices. The optional tangent space
// 		data (normal, tangent, bitangent) is encoded as int16, where -1.0 maps to
// 		-32'767 and 1.0f to +32'767.

// 	Parameters:
// 		geoName            - unique name of the new Geometry resource
// 		numVertices        - number of vertices
// 		numTriangleIndices - number of vertex indices
// 		posData            - vertex positions (xyz)
// 		indexData          - indices defining triangles
// 		normalData         - normals xyz (optional, can be NULL)
// 		tangentData        - tangents xyz (optional, can be NULL)
// 		bitangentData      - bitangents xyz (required if tangents specified, otherwise NULL)
// 		texData1           - first texture coordinate uv set (optional, can be NULL)
// 		texData2           - second texture coordinate uv set (optional, can be NULL)

// 	Returns:
// 		handle to new Geometry resource or 0 in case of failure

H3DNode CreateProceduralGeometry(const char* geoName, float* vertices, unsigned int* indices,
                                 // Optional
                                 short* normals, short* tangents, short* bitangents,
                                 float* texture1UVs, float* texture2UVs,
                                 // Required
                                 int numVertices, int numIndices);
