#include "DebugDisplay.hpp"

#include "Logging.hpp"

namespace DebugDisplay
{
window* outputWindow = nullptr;
text displayText;
const float displayTextSize = 28.f;
const float lineSpacing = 2.f;
const float startOffset = 5.f;

float displayYOffset = startOffset;

std::vector<std::string> outputBuffer;

void initialize(window* newOutputWindow)
{
	if (!displayText.loadFont("data/fonts/UbuntuMono-R.ttf"))
	{
		LOGE << "Error: Cannot load default text font. Nothing will work";
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
			LOGE
			    << "ERROR: DebugDisplay::print() called before output window was set!\n Message:"
			    << "\t" << str;
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

void clear()
{
	outputBuffer.clear();
}
}  // namespace DebugDisplay
