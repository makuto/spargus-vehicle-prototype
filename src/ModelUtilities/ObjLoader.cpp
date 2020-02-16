#include "ObjLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION  // define this in only *one* .cc
#include "tiny_obj_loader.h"

#include "Performance.hpp"
#include "PhysicsWorld.hpp"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include <iostream>

bool objTest()
{
	std::string inputfile = "assets/TruePlane.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputfile.c_str());

	if (!warn.empty())
	{
		std::cout << warn << std::endl;
	}

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		return false;
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

				std::cout << vx << ", " << vy << ", " << vz << "\n";
				std::cout << idx.vertex_index << " offset " << index_offset << "\n";
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}
	}

	return true;
}

bool objToBulletTriangleMesh(PhysicsWorld& world, const char* filename)
{
	PerfTimeNamedScope(objToBulletScope, "Load Collision .Obj to Bullet", tracy::Color::SteelBlue3);
	PerfSetNameFormat(objToBulletScope, "Load collision obj '%s'", filename);

	const bool debugPrint = false;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);

	if (!warn.empty())
	{
		std::cout << warn << std::endl;
	}

	if (!err.empty())
	{
		std::cerr << err << std::endl;
	}

	if (!ret)
	{
		return false;
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// TODO: Macoy: Memory leak!
		std::vector<btVector3>* bulletVertices = new std::vector<btVector3>;
		std::vector<int>* bulletIndicies = new std::vector<int>;

		// Loop over faces(polygon)
		size_t index_offset = 0;
		size_t numFaces = shapes[s].mesh.num_face_vertices.size();
		for (size_t f = 0; f < numFaces; f++)
		{
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

				// std::cout << vx << ", " << vy << ", " << vz << "\n";

				btVector3 vertex = {vx, vy, vz};
				// Make room (TODO: Preallocate)
				if (idx.vertex_index >= bulletVertices->size())
					bulletVertices->resize(idx.vertex_index + 1);
				(*bulletVertices)[idx.vertex_index] = vertex;
				bulletIndicies->push_back(idx.vertex_index);
			}
			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];
		}

		if (debugPrint)
		{
			for (const btVector3& vertex : *bulletVertices)
			{
				std::cout << vertex.x() << ", " << vertex.y() << ", " << vertex.z() << "\n";
			}

			for (int index : *bulletIndicies)
			{
				std::cout << index << "\n";
			}

			std::cout << "NumFaces: " << numFaces << "\n";
			std::cout << "Num Vertices: " << bulletVertices->size() << "\n";
		}
		// Add shape to world
		const int vertStride = sizeof(btVector3);
		const int indexStride = 3 * sizeof(int);

		if (debugPrint)
			std::cout << "Vert stride: " << vertStride << " Index stride: " << indexStride << "\n";

		btTriangleIndexVertexArray* indexVertexArrays = new btTriangleIndexVertexArray(
		    numFaces, &(*bulletIndicies)[0], indexStride, bulletVertices->size(),
		    (btScalar*)&(*bulletVertices)[0].x(), vertStride);

		bool useQuantizedAabbCompression = true;

		// TODO: Macoy: "The btTriangleIndexVertexArray allows to access multiple triangle meshes,
		// by indexing into existing triangle/index arrays. Additional meshes can be added using
		// addIndexedMesh"
		btCollisionShape* shape =
		    new btBvhTriangleMeshShape(indexVertexArrays, useQuantizedAabbCompression);
		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(0.0, 0.0, 0.0));
		world.localCreateRigidBody(PhysicsWorld::StaticRigidBodyMass, transform, shape);
		world.world->updateAabbs();
	}

	return true;
}
