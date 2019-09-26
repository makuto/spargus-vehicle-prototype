#pragma once

// Just use a slick single-header library for most of this
#include "gb_math.h"

class btTransform;
void PositionRotationFromBulletTransform(const btTransform& transform, float* vPositionOut,
                                         float* qRotationOut);
