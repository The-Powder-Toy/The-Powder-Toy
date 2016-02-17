#ifndef RENDER_PRESET_H
#define RENDER_PRESET_H
#include "Format.h"
class RenderPreset
{
public:
	std::wstring Name;
	std::vector<unsigned int> RenderModes;
	std::vector<unsigned int> DisplayModes;
	unsigned int ColourMode;

	RenderPreset(): Name(L""), ColourMode(0) {}
	RenderPreset(std::string name, std::vector<unsigned int> renderModes, std::vector<unsigned int> displayModes, unsigned int colourMode):
		Name(format::StringToWString(name)),
		RenderModes(renderModes),
		DisplayModes(displayModes),
		ColourMode(colourMode)
	{}
	RenderPreset(std::wstring name, std::vector<unsigned int> renderModes, std::vector<unsigned int> displayModes, unsigned int colourMode):
		Name(name),
		RenderModes(renderModes),
		DisplayModes(displayModes),
		ColourMode(colourMode)
	{}
};
#endif
