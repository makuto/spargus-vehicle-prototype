#pragma once

#include "mesh.h"

class btDiscreteDynamicsWorld;

void BulletMeshFromGltfMesh(const gltf::Mesh<float>& mesh, btDiscreteDynamicsWorld* dynamicsWorld);
