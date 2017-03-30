#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PINVIS PT_PINVIS 192
Element_PINVIS::Element_PINVIS()
{
	Identifier = "DEFAULT_PT_PINVIS";
	Name = "PINV";
	Colour = PIXPACK(0x00CCCC);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 164;
	Description = "Powered invisible, invisible to particles while activated.";

	Properties = TYPE_SOLID | PROP_NEUTPASS | PROP_NOSLOWDOWN | PROP_TRANSPARENT | PROP_INVISIBLE | PROP_NODESTRUCT;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PINVIS::update;
	Graphics = &Element_INVIS::graphics;
}

//#TPT-Directive ElementHeader Element_PINVIS static int update(UPDATE_FUNC_ARGS)
int Element_PINVIS::update(UPDATE_FUNC_ARGS)
{
#if 0
// test failed
	int r, rx, ry;
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_PINVIS)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
	return 0;
#endif

	// wireless2[][0] - whether channel is active on this frame
	// for wireless2[][1] - see SPRK.cpp and Simulation.cpp

	parts[i].ctype &= 0x0FFF;
	int PINVIS_channel = parts[i].ctype;
	parts[i].tmp = 1 & (sim->wireless2[PINVIS_channel >> 5][0] >> (PINVIS_channel & 0x1F));
	return 0;
}


Element_PINVIS::~Element_PINVIS() {}
