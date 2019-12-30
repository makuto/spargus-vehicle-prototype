#pragma once

#include "graphics/graphics.hpp"

namespace DebugDisplay
{
extern text displayText;

void initialize(window* newOutputWindow);
void print(std::string output);
void endFrame();
void clear();
}  // namespace DebugDisplay
