#include "Terrain.hpp"

#include <algorithm>

#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "btBulletDynamicsCommon.h"

#include "noise/noise.hpp"

#include "Logging.hpp"
#include "Math.hpp"
#include "PhysicsWorld.hpp"

// Important: Note that even if the type is called PHY_"FLOAT", this needs to be btScalar (i.e. can
// end up being a double)
typedef btScalar HeightfieldCell;

static int s_gridSize = 20 + 1;  // must be (2^N) + 1
// static int s_gridSize = 16 + 1;  // must be (2^N) + 1
// static int s_gridSize = 8 + 1;  // must be (2^N) + 1
static btScalar s_gridSpacing = 1.0;
static const int upAxis = 1;

static const int worldSeed = 51388234;
static Noise2d noiseGenerator(worldSeed);

// TODO Memory leaks
void createCollisionHeightfield(PhysicsWorld& world)
{
	// get new heightfield of appropriate type
	// rawHeightfieldData =
	// getRawHeightfieldData(model, type, minHeight, maxHeight);
	// btAssert(rawHeightfieldData && "failed to create raw heightfield");
	
	float xOffset = 0.f;
	float zOffset = 0.f;

	float minY = 0.f;
	float maxY = 0.f;
	HeightfieldCell* rawHeightfieldData = new HeightfieldCell[s_gridSize * s_gridSize];
	for (int cellZ = 0; cellZ < s_gridSize; cellZ++)
	{
		for (int cellX = 0; cellX < s_gridSize; cellX++)
		{
			float noiseScale = 0.5f;
			float noiseX = (cellX + xOffset) * noiseScale;
			float noiseZ = (cellZ + zOffset) * noiseScale;

			float value = noiseGenerator.scaledOctaveNoise2d(noiseX, noiseZ, 0.f, 10.f, 10,
			                                                 0.1f, 0.55f, 2);

			rawHeightfieldData[(cellZ * s_gridSize) + cellX] = value;
			minY = std::min(minY, value);
			maxY = std::max(maxY, value);
		}
	}

	bool flipQuadEdges = false;
	const PHY_ScalarType heightfieldDataType = PHY_FLOAT;
	// HeightScale is ignored when using the float heightfield data type.
	btScalar gridHeightScale = 1.0;
	btHeightfieldTerrainShape* heightfieldShape = new btHeightfieldTerrainShape(
	    s_gridSize, s_gridSize, rawHeightfieldData, gridHeightScale, minY, maxY, upAxis,
	    heightfieldDataType, flipQuadEdges);

	// buildAccelerator is optional, it may not support all features.
	// Builds a grid data structure storing the min and max heights of the terrain in chunks.
	// if chunkSize is zero, that accelerator is removed.
	// If you modify the heights, you need to rebuild this accelerator.
	heightfieldShape->buildAccelerator();

	btScalar scaleHeightfield = 5.0;
	heightfieldShape->setLocalScaling(
	    btVector3(scaleHeightfield, scaleHeightfield, scaleHeightfield));

	// Note use
	// void btHeightfieldTerrainShape::processAllTriangles(btTriangleCallback* callback, const
	// btVector3& aabbMin, const btVector3& aabbMax) const
	// To generate visible triangles

	// set origin to middle of heightfield
	btTransform transform;
	transform.setIdentity();
	// Note that this origin will be center of heightfield, so it will need to be adjusted if bounds change
	transform.setOrigin(btVector3(0, 0, -4));
	btRigidBody* body =
	    world.localCreateRigidBody(PhysicsWorld::StaticRigidBodyMass, transform, heightfieldShape);
}
