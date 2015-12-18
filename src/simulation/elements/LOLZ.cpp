#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LOLZ PT_LOLZ 123
Element_LOLZ::Element_LOLZ()
{
	Identifier = "DEFAULT_PT_LOLZ";
	Name = "LOLZ";
	Colour = PIXPACK(0x569212);
	MenuVisible = 1;
	MenuSection = SC_CRACKER2;
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

	Temperature = 373.0f;
	HeatConduct = 40;
	Description = "Lolz";

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

//#TPT-Directive ElementHeader Element_LOLZ static int RuleTable[9][9]
int Element_LOLZ::RuleTable[9][9] =
{
	{0,0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,0,0,0,1,0,0},
	{1,0,0,1,1,0,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{1,0,1,0,0,1,0,1,0},
	{0,1,0,1,1,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
	{0,1,0,0,0,0,0,1,0},
};

//#TPT-Directive ElementHeader Element_LOLZ static int lolz[XRES/9][YRES/9];
int Element_LOLZ::lolz[XRES/9][YRES/9];

Element_LOLZ::~Element_LOLZ() {}
