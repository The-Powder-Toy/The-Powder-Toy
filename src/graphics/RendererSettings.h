#pragma once
#include "gui/interface/Point.h"
#include "simulation/ElementGraphics.h"
#include "simulation/ElementDefs.h"
#include "FindingElement.h"
#include <cstdint>
#include <optional>
#include <variant>

struct HdispLimitExplicit
{
	float value;
};
struct HdispLimitAuto
{
};
using HdispLimit = std::variant<
	HdispLimitExplicit,
	HdispLimitAuto
>;

struct RendererSettings
{
	uint32_t renderMode = RENDER_BASC | RENDER_FIRE | RENDER_SPRK | RENDER_EFFE;
	uint32_t displayMode = 0;
	uint32_t colorMode = COLOUR_DEFAULT;
	std::optional<FindingElement> findingElement;
	bool gravityZonesEnabled = false;
	bool gravityFieldEnabled = false;
	enum DecorationLevel
	{
		decorationDisabled,
		decorationEnabled,
		decorationAntiClickbait,
	};
	DecorationLevel decorationLevel = decorationEnabled;
	bool debugLines = false;
	ui::Point mousePos = { 0, 0 };
	int gridSize = 0;
	float fireIntensity = 1;
	HdispLimit wantHdispLimitMin = HdispLimitExplicit{ MIN_TEMP };
	HdispLimit wantHdispLimitMax = HdispLimitExplicit{ MAX_TEMP };
	Rect<int> autoHdispLimitArea = RES.OriginRect();
};
