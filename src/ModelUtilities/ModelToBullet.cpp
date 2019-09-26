#include "ModelToBullet.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

// This is an abomination
// TODO It leaks memory
// From bullet3/examples/SoftDemo/SoftDemo.cpp (with modifications)
void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, btDiscreteDynamicsWorld* dynamicsWorld)
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

	btCollisionObject* newOb = new btCollisionObject();
	btTransform tr;
	tr.setIdentity();
	tr.setOrigin(btVector3(0, 0, 0));
	newOb->setWorldTransform(tr);
	newOb->setInterpolationWorldTransform(tr);
	newOb->setCollisionShape(groundShape);
	dynamicsWorld->addCollisionObject(newOb);
}
