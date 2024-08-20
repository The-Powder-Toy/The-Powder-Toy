#include "simulation/ElementCommon.h"
#include "graphics/VideoBuffer.h"

static std::unique_ptr<VideoBuffer> iconGen(int wallID, Vec2<int> size);

void Element::Element_NONE()
{
	Identifier = "DEFAULT_PT_NONE";
	Name = "NONE";
	Colour = 0x000000_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "Erases particles.";

	Properties = 0;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	IconGenerator = &iconGen;
}

static std::unique_ptr<VideoBuffer> iconGen(int wallID, Vec2<int> size)
{
	auto texture = std::make_unique<VideoBuffer>(size);
	texture->BlendChar(size / 2 - Vec2(4, 2), 0xE06C, 0xFF0000_rgb .WithAlpha(0xFF));
	return texture;
}
