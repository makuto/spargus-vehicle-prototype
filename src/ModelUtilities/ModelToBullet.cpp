#include "ModelToBullet.hpp"

#include "PhysicsWorld.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "btBulletWorldImporter.h"

// This is an abomination
// TODO It leaks memory
// From bullet3/examples/SoftDemo/SoftDemo.cpp (with modifications)
void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, PhysicsWorld& world)
{
	size_t totalVerts = mesh.vertices.size() / 3;
	size_t totalTriangles = mesh.vertices.size() / 3 / 3;
	btCollisionShape* groundShape = nullptr;
	{
		// int i;
		// int j;

		// const int NUM_VERTS_X = 30;
		// const int NUM_VERTS_Y = 30;
		// const int totalVerts = NUM_VERTS_X * NUM_VERTS_Y;
		// const int totalTriangles = 2 * (NUM_VERTS_X - 1) * (NUM_VERTS_Y - 1);

		// // TODO wtf
		static btVector3* gGroundVertices = new btVector3[totalVerts];
		static int* gGroundIndices = new int[totalTriangles * 3];

		for (size_t i = 0; i < mesh.vertices.size(); i += 3)
		{
			for (size_t coord = 0; coord < 3; ++coord)
				gGroundVertices[i / 3][coord] = mesh.vertices[i + coord];
			gGroundIndices[i / 3] = i / 3;
		}

		int vertStride = sizeof(btVector3);
		int indexStride = 3 * sizeof(int);

		// int index = 0;
		// for (i = 0; i < NUM_VERTS_X - 1; i++)
		// {
		// 	for (int j = 0; j < NUM_VERTS_Y - 1; j++)
		// 	{
		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i + 1;
		// 		;

		// 		gGroundIndices[index++] = j * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i;
		// 		gGroundIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
		// 	}
		// }

		btTriangleIndexVertexArray* indexVertexArrays =
		    new btTriangleIndexVertexArray(totalTriangles, gGroundIndices, indexStride, totalVerts,
		                                   (btScalar*)&gGroundVertices[0].x(), vertStride);

		bool useQuantizedAabbCompression = true;

		groundShape = new btBvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);
		groundShape->setMargin(0.5);
	}

	// This only handles collision. It must be a rigid body to receive raycasts etc.
	// btCollisionObject* newOb = new btCollisionObject();
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(0.0, 0.0, 0.0));
	// newOb->setWorldTransform(transform);
	// newOb->setInterpolationWorldTransform(transform);
	// newOb->setCollisionShape(groundShape);

	world.localCreateRigidBody(PhysicsWorld::StaticRigidBodyMass, transform, groundShape);
	
	world.world->updateAabbs();
}

//
// From Bullet3 concave demo (modified)
//
void setVertexPositions(btVector3* gVertices, int* gIndices, int NUM_VERTS_X, int NUM_VERTS_Y,
                        float TRIANGLE_SIZE, float waveheight, float offset)
{
	int i;
	int j;

	for (i = 0; i < NUM_VERTS_X; i++)
	{
		for (j = 0; j < NUM_VERTS_Y; j++)
		{
			gVertices[i + j * NUM_VERTS_X].setValue(
			    (i - NUM_VERTS_X * 0.5f) * TRIANGLE_SIZE,
			    // 0.f,
			    waveheight * sinf((float)i + offset) * cosf((float)j + offset),
			    (j - NUM_VERTS_Y * 0.5f) * TRIANGLE_SIZE);
		}
	}
}

void serializeConcaveMesh()
{
	static btVector3* gVertices = 0;
	static int* gIndices = 0;
	const int NUM_VERTS_X = 30;
	const int NUM_VERTS_Y = 30;
	const float TRIANGLE_SIZE = 8.f;
	const int totalVerts = NUM_VERTS_X * NUM_VERTS_Y;
	static float waveheight = 5.f;
	static btBvhTriangleMeshShape* trimeshShape = 0;
	const char* outputFile = "ConcaveMesh.bullet";

	int vertStride = sizeof(btVector3);
	int indexStride = 3 * sizeof(int);

	const int totalTriangles = 2 * (NUM_VERTS_X - 1) * (NUM_VERTS_Y - 1);

	gVertices = new btVector3[totalVerts];
	gIndices = new int[totalTriangles * 3];

	int i;

	setVertexPositions(gVertices, gIndices, NUM_VERTS_X, NUM_VERTS_Y, TRIANGLE_SIZE, waveheight, 0.f);

	int index = 0;
	for (i = 0; i < NUM_VERTS_X - 1; i++)
	{
		for (int j = 0; j < NUM_VERTS_Y - 1; j++)
		{
			gIndices[index++] = j * NUM_VERTS_X + i;
			gIndices[index++] = j * NUM_VERTS_X + i + 1;
			gIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;

			gIndices[index++] = j * NUM_VERTS_X + i;
			gIndices[index++] = (j + 1) * NUM_VERTS_X + i + 1;
			gIndices[index++] = (j + 1) * NUM_VERTS_X + i;
		}
	}

	btTriangleIndexVertexArray* m_indexVertexArrays;
	m_indexVertexArrays =
	    new btTriangleIndexVertexArray(totalTriangles, gIndices, indexStride, totalVerts,
	                                   (btScalar*)&gVertices[0].x(), vertStride);

	bool useQuantizedAabbCompression = true;

	btVector3 aabbMin(-1000, -1000, -1000), aabbMax(1000, 1000, 1000);

	trimeshShape = new btBvhTriangleMeshShape(m_indexVertexArrays, useQuantizedAabbCompression,
	                                          aabbMin, aabbMax);
	// TODO: Macoy: Memory leak
	// m_collisionShapes.push_back(trimeshShape);

	int maxSerializeBufferSize = 1024 * 1024 * 5;
	btDefaultSerializer* serializer = new btDefaultSerializer(maxSerializeBufferSize);
	// serializer->setSerializationFlags(BT_SERIALIZE_NO_BVH);//	or
	// BT_SERIALIZE_NO_TRIANGLEINFOMAP
	serializer->startSerialization();
	// registering a name is optional, it allows you to retrieve the shape by name
	// serializer->registerNameForPointer(trimeshShape,"mymesh");
#define SERIALIZE_SHAPE
#ifdef SERIALIZE_SHAPE
	trimeshShape->serializeSingleShape(serializer);
#else
	trimeshShape->serializeSingleBvh(serializer);
#endif
	serializer->finishSerialization();
	FILE* f2 = fopen(outputFile, "wb");
	fwrite(serializer->getBufferPointer(), serializer->getCurrentBufferSize(), 1, f2);
	fclose(f2);
}

btCollisionShape* importConcaveMesh()
{
	const char* outputFile = "ConcaveMesh.bullet";
	static btBvhTriangleMeshShape* trimeshShape = nullptr;

	// Init vertex array buffer
	btTriangleIndexVertexArray* m_indexVertexArrays;
	{
		const int NUM_VERTS_X = 30;
		const int NUM_VERTS_Y = 30;

		const int totalTriangles = 2 * (NUM_VERTS_X - 1) * (NUM_VERTS_Y - 1);
		const int totalVerts = NUM_VERTS_X * NUM_VERTS_Y;
		
		// Must be static due to scope
		static btVector3* gVertices = nullptr;
		{
			gVertices = new btVector3[totalVerts];
		}
		static int* gIndices = nullptr;
		{
			gIndices = new int[totalTriangles * 3];
		}

		int vertStride = sizeof(btVector3);
		int indexStride = 3 * sizeof(int);
		m_indexVertexArrays =
		    new btTriangleIndexVertexArray(totalTriangles, gIndices, indexStride, totalVerts,
		                                   (btScalar*)&gVertices[0].x(), vertStride);
	}

	btBulletWorldImporter import(0);  // don't store info into the world
	if (import.loadFile(outputFile))
	{
		int numBvh = import.getNumBvhs();
		if (numBvh)
		{
			btOptimizedBvh* bvh = import.getBvhByIndex(0);
			btVector3 aabbMin(-1000, -1000, -1000), aabbMax(1000, 1000, 1000);
			bool useQuantizedAabbCompression = true;
			
			trimeshShape = new btBvhTriangleMeshShape(
			    m_indexVertexArrays, useQuantizedAabbCompression, aabbMin, aabbMax, false);
			trimeshShape->setOptimizedBvh(bvh);
			// trimeshShape  = new
			// btBvhTriangleMeshShape(m_indexVertexArrays,useQuantizedAabbCompression,aabbMin,aabbMax);
			// trimeshShape->setOptimizedBvh(bvh);
		}
		int numShape = import.getNumCollisionShapes();
		if (numShape)
		{
			trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByIndex(0);

			// if you know the name, you can also try to get the shape by name:
			const char* meshName = import.getNameForPointer(trimeshShape);
			if (meshName)
				trimeshShape = (btBvhTriangleMeshShape*)import.getCollisionShapeByName(meshName);

			return trimeshShape;
		}
	}
	
	return nullptr;
}
