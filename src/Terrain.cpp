#include "Terrain.hpp"

#include <algorithm>
#include <cstdio>  //  sprintf
#include <limits>

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "btBulletDynamicsCommon.h"

#include <glm/vec3.hpp>  // vec3

#include "noise/noise.hpp"

#include "Color.hpp"
#include "DebugDraw.hpp"
#include "GraphicsObject.hpp"
#include "Logging.hpp"
#include "Math.hpp"
#include "Performance.hpp"
#include "PhysicsWorld.hpp"

// This must be crap is probably not true I think
int g_TerrainGridSize = 20 + 1;  // must be (2^N) + 1
float scaleHeightfield = 5.0f;

// static int g_TerrainGridSize = 16 + 1;  // must be (2^N) + 1
// static int g_TerrainGridSize = 8 + 1;  // must be (2^N) + 1
// static btScalar s_gridSpacing = 1.0;
static const int upAxis = 1;

static const int worldSeed = 51388234;
static Noise2d noiseGenerator(worldSeed);

class TerrainTriangleCollector : public btTriangleCallback
{
public:
	std::vector<float>* VerticesOut;
	std::vector<unsigned int>* IndicesOut;
	std::vector<float>* NormalsOut;
	std::vector<float>* UVsOut;

	TerrainTriangleCollector()
	{
		VerticesOut = nullptr;
		IndicesOut = nullptr;
	}

	// TODO: What a waste of vertices. Is there duplication happening? (look at HeightfieldExample)
	virtual void processTriangle(btVector3* vertices, int partId, int triangleIndex)
	{
		for (int vertexIndex = 0; vertexIndex < 3; vertexIndex++)
		{
			// Keeping this in bullet vector math to be fast
			btVector3 normal = (vertices[0] - vertices[1]).cross(vertices[0] - vertices[2]);
			normal.safeNormalize();
			IndicesOut->push_back(VerticesOut->size() / 3);

			for (int iAxis = 0; iAxis < 3; iAxis++)
			{
				VerticesOut->push_back(vertices[vertexIndex][iAxis]);
				NormalsOut->push_back(normal[iAxis]);
			}

			// TODO Pretty sure these are wrong
			switch (vertexIndex)
			{
				case 0:
					UVsOut->push_back(0.f);  // U
					UVsOut->push_back(0.f);  // V
					break;
				case 1:
					UVsOut->push_back(1.f);  // U
					UVsOut->push_back(0.f);  // V
					break;
				case 2:
					UVsOut->push_back(1.f);  // U
					UVsOut->push_back(1.f);  // V
					break;
			}
		}
	}
};

// TODO Memory leaks
// Note that heightfield data is not owned by Bullet
void createCollisionHeightfield(PhysicsWorld& world, const glm::vec3& worldPosition)
{
	PerfTimeNamedScope(terrainInit, "Terrain creation", tracy::Color::LawnGreen);

	// Get new heightfield of appropriate type
	// g_rawHeightfieldData =
	// getRawHeightfieldData(model, type, minHeight, maxHeight);
	// btAssert(g_rawHeightfieldData && "failed to create raw heightfield");

	// Do noise in unscaled space so that changing the scale doesn't necessitate a change of neise
	const float xNoiseSpaceOffset = worldPosition[0] / scaleHeightfield;
	const float zNoiseSpaceOffset = worldPosition[2] / scaleHeightfield;

	// TODO: Leak!
	HeightfieldCell* rawHeightfieldData =
	    new HeightfieldCell[g_TerrainGridSize * g_TerrainGridSize];

	float minY = 0.f;
	float maxY = 0.f;
	{
		PerfTimeNamedScope(terrainNoise, "Terrain noise generation", tracy::Color::ForestGreen);

		for (int cellZ = 0; cellZ < g_TerrainGridSize; cellZ++)
		{
			for (int cellX = 0; cellX < g_TerrainGridSize; cellX++)
			{
				float noiseScale = 0.7f;
				float noiseX = (cellX + xNoiseSpaceOffset) * noiseScale;
				float noiseZ = (cellZ + zNoiseSpaceOffset) * noiseScale;

				float value = noiseGenerator.scaledOctaveNoise2d(noiseX, noiseZ, 0.f, 10.f, 4, 0.1f,
				                                                 0.22f, 2.f);

				rawHeightfieldData[(cellZ * g_TerrainGridSize) + cellX] = value;
				minY = std::min(minY, value);
				maxY = std::max(maxY, value);
			}
		}
	}

	bool flipQuadEdges = false;
	const PHY_ScalarType heightfieldDataType = PHY_FLOAT;
	// HeightScale is ignored when using the float heightfield data type.
	btScalar gridHeightScale = 1.f;
	btHeightfieldTerrainShape* heightfieldShape = new btHeightfieldTerrainShape(
	    g_TerrainGridSize, g_TerrainGridSize, rawHeightfieldData, gridHeightScale, minY, maxY,
	    upAxis, heightfieldDataType, flipQuadEdges);

	// BuildAccelerator is optional, it may not support all features.
	// Builds a grid data structure storing the min and max heights of the terrain in chunks.
	// if chunkSize is zero, that accelerator is removed.
	// If you modify the heights, you need to rebuild this accelerator.
	heightfieldShape->buildAccelerator();

	heightfieldShape->setLocalScaling(
	    btVector3(scaleHeightfield, scaleHeightfield, scaleHeightfield));

	// Set origin to middle of heightfield
	btTransform transform;
	transform.setIdentity();
	// Bullet computes the origin of the heightfield to be in the center. Because the geometry is
	// procedural, we need to adjust its transform up or down based on where the origin ended up
	glm::vec3 adjustedOrigin(worldPosition);
	adjustedOrigin[1] += ((maxY - minY) / 2.f) * scaleHeightfield;
	transform.setOrigin(glmVec3ToBulletVector(adjustedOrigin));
	// btRigidBody* body =
	world.localCreateRigidBody(PhysicsWorld::StaticRigidBodyMass, transform, heightfieldShape);

	// Rendering: create render geo from the heightfield
	{
		TerrainTriangleCollector triangleCollector;
		std::vector<float> vertices;
		std::vector<unsigned int> indices;
		std::vector<float> normals;
		std::vector<float> uvs;
		triangleCollector.VerticesOut = &vertices;
		triangleCollector.IndicesOut = &indices;
		triangleCollector.NormalsOut = &normals;
		triangleCollector.UVsOut = &uvs;

		btVector3 aabbMin, aabbMax;
		for (int k = 0; k < 3; k++)
		{
			aabbMin[k] = -std::numeric_limits<float>::max();
			aabbMax[k] = std::numeric_limits<float>::max();
		}

		{
			PerfTimeNamedScope(terrainNoise, "Terrain triangle collection",
			                   tracy::Color::MediumSpringGreen);

			heightfieldShape->processAllTriangles(&triangleCollector, aabbMin, aabbMax);
		}

		if (vertices.size() && indices.size())
		{
			// Graphics::TestProceduralGeometry("Heightfield", vertices.data(),
			//                                  indices.data(), vertices.size() / 3,
			//                                  indices.size());

			Graphics::ProceduralMesh graphicsMesh;
			char nameBuffer[64];
			snprintf(nameBuffer, sizeof(nameBuffer), "Heightfield_%d_%d", (int)worldPosition[0],
			         (int)worldPosition[2]);
			graphicsMesh.Initialize(nameBuffer, vertices.data(), indices.data(), nullptr, nullptr,
			                        nullptr, uvs.data(), nullptr, vertices.size() / 3,
			                        indices.size());
			// Note that scaling happens in processAllTriangles(), not from the transform
			graphicsMesh.SetTransform(BulletTransformToGlmMat4(transform));

			// Doesn't use indices! Probably wrong
			// for (int i = 0; i < vertices.size(); i += (3 * 2))
			// {
			// 	glm::vec3 from(vertices[i + 0], vertices[i + 1], vertices[i + 2]);
			// 	glm::vec3 to(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
			// 	DebugDraw::addLine(from, to, Color::Green, Color::Red,
			// 	                   DebugDraw::Lifetime_VeryLongTime);
			// }
		}
	}
}
