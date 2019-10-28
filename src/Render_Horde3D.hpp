#pragma once

typedef int H3DNode;
typedef int H3DRes;
extern H3DNode hordeCamera;
extern H3DNode buggyNode;
extern H3DNode buggyWheelNodes[4];

void hordeInitialize(int winWidth, int winHeight);
void hordeTestInitialize(int winWidth, int winHeight);

void hordeUpdate(float fps);
void hordeRelease();
