#include "DebugDisplay.hpp"

#include <iostream>

namespace DebugDisplay
{
window* outputWindow = nullptr;
text displayText;
const float displayTextSize = 32.f;
const float lineSpacing = 2.f;
const float startOffset = 5.f;

float displayYOffset = startOffset;

std::vector<std::string> outputBuffer;

void initialize(window* newOutputWindow)
{
	if (!displayText.loadFont("data/fonts/UbuntuMono-R.ttf"))
	{
		std::cout << "Error: Cannot load default text font. Nothing will work\n";
		return;
	}

	displayText.setSize(32);
	displayText.setColor(232, 30, 34, 255);

	outputWindow = newOutputWindow;
}

void print(std::string output)
{
	outputBuffer.push_back(output);
}

void endFrame()
{
	for (const std::string& str : outputBuffer)
	{
		if (!outputWindow)
		{
			std::cout
			    << "ERROR: DebugDisplay::print() called before output window was set!\n Message:"
			    << "\t" << str << "\n";
			return;
		}

		DebugDisplay::displayText.setText(str);
		DebugDisplay::displayText.setPosition(5.f, displayYOffset);
		outputWindow->draw(&DebugDisplay::displayText);
		displayYOffset += displayTextSize + lineSpacing;
	}

	outputBuffer.clear();

	displayYOffset = startOffset;
}
}  // namespace DebugDisplay
