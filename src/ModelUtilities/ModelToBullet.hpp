#pragma once

#include "mesh.h"

struct PhysicsWorld;

void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, PhysicsWorld& world);
