#include "simulation/ElementCommon.h"
#include "PAPR.h"

// Element overview:
// PAPR (Paper) is a flammable solid element that can be colored by certain other elements.
// Additionally, it can be read and written to by ARAY and LDTC.

// Property usage:
// life: Whether or not the particle is marked
// tmp: Temporary read/write state for ARAY interaction
// tmp2: Singe level

void Element::Element_EPPR()
{
	Identifier = "DEFAULT_PT_EPPR";
	Name = "EPPR";
	Colour = 0xCADFCF_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.995f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 60;

	Weight = 100;

	HeatConduct = 80;
	Description = "Electronic paper. Use PSCN to make it solid, NSCN to make it pass-through.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 700.0f;
	HighTemperatureTransition = PT_NONE;

	Update = &Element_PAPR_update;
	Graphics = &Element_PAPR_graphics;
	CtypeDraw = &Element_PAPR_cTypeDraw;
}