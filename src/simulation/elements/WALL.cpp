#include "simulation/ElementCommon.h"

void Element::Element_WALL()
{
	Identifier = "DEFAULT_PT_WALL";
	Name = "WALL";
	Colour = PIXPACK(0x808080);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	DefaultProperties.temp = 373.0f;
	HeatConduct = 40;
	Description = "WALL, now in elements.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
}

int Element_WALL_RuleTable[9][9] =
{
	{0,1,1,1,1,1,1,1,0},
	{1,1,0,0,0,0,0,1,1},
	{1,0,1,0,0,0,1,0,1},
	{1,0,0,1,0,1,0,0,1},
	{1,0,0,0,1,0,0,0,1},
	{1,0,0,1,0,1,0,0,1},
	{1,0,1,0,0,0,1,0,1},
	{1,1,0,0,0,0,0,1,1},
	{0,1,1,1,1,1,1,1,0},

};

int Element_WALL_wall[XRES / 9][YRES / 9];
