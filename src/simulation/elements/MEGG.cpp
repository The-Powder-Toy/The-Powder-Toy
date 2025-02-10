#include "simulation/ElementCommon.h"
#include "MEGG.h"
void Element::Element_MEGG()
{
	Identifier = "DEFAULT_PT_MEGG";
	Name = "MEGG";
	Colour = 0xF0F0F0_rgb;
	MenuVisible = 0;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;
	PhotonReflectWavelengths = 0x3FFFFFC0;

	Weight = 85;

	HeatConduct = 70;
	Description = "Moth egg.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 288.15f;
	LowTemperatureTransition = PT_DUST;
	HighTemperature = 318.15f;
	HighTemperatureTransition = PT_DUST;

	Graphics = &Element_MEGG_graphics;
	Update = &Element_MEGG_update;
}
int Element_MEGG_graphics(GRAPHICS_FUNC_ARGS)
{
	return 0;
}

int Element_MEGG_update(UPDATE_FUNC_ARGS)
{
	sim->parts[i].life += 1;
	if (parts[i].life > 600)
	{
		sim->kill_part(i);
		sim->create_part(-1, x, y, PT_MOTH);
	}
	return 0;
}
