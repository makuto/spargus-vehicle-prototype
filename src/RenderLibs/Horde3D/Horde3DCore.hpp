#pragma once

#include <Horde3D.h>

extern bool g_graphicsIntialized;

H3DNode TestProceduralGeometry(const char* geoName, float* vertices, unsigned int* indices,
                               int numTriangles, int numIndices);
