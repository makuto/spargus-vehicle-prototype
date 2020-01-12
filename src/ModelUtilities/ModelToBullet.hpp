#pragma once

#include "mesh.h"

class PhysicsWorld;

void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, PhysicsWorld& world);

void serializeConcaveMesh();

class btCollisionShape;
btCollisionShape* importConcaveMesh();
