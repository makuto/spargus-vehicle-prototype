#pragma once

typedef int H3DNode;
extern H3DNode hordeCamera;
extern H3DNode buggyNode;

void hordeInitialize(int winWidth, int winHeight);
void hordeTestInitialize(int winWidth, int winHeight);

void hordeUpdate(float fps);
void hordeRelease();
