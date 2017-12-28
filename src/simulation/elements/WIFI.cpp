#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WIFI PT_WIFI 124
Element_WIFI::Element_WIFI()
{
	Identifier = "DEFAULT_PT_WIFI";
	Name = "WIFI";
	Colour = PIXPACK(0x40A060);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Hardness = 2;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Wireless transmitter, transfers spark to any other wifi on the same temperature channel.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 15.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_WIFI::update;
	Graphics = &Element_WIFI::graphics;
}

//#TPT-Directive ElementHeader Element_WIFI static int update(UPDATE_FUNC_ARGS)
int Element_WIFI::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				// wireless[][0] - whether channel is active on this frame
				// wireless[][1] - whether channel should be active on next frame
				if (sim->wireless[parts[i].tmp][0])
				{
					if ((TYP(r)==PT_NSCN||TYP(r)==PT_PSCN||TYP(r)==PT_INWR)&&parts[ID(r)].life==0 && sim->wireless[parts[i].tmp][0])
					{
						parts[ID(r)].ctype = TYP(r);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
						parts[ID(r)].life = 4;
					}
				}
				if (TYP(r)==PT_SPRK && parts[ID(r)].ctype!=PT_NSCN && parts[ID(r)].life>=3)
				{
					sim->wireless[parts[i].tmp][1] = 1;
					sim->ISWIRE = 2;
				}
			}
	return 0;
}

#define FREQUENCY 0.0628f

//#TPT-Directive ElementHeader Element_WIFI static int graphics(GRAPHICS_FUNC_ARGS)
int Element_WIFI::graphics(GRAPHICS_FUNC_ARGS)
{
	int q = (int)((cpart->temp-73.15f)/100+1);
	*colr = sin(FREQUENCY*q + 0) * 127 + 128;
	*colg = sin(FREQUENCY*q + 2) * 127 + 128;
	*colb = sin(FREQUENCY*q + 4) * 127 + 128;
	*pixel_mode |= EFFECT_DBGLINES;
	return 0;
}


Element_WIFI::~Element_WIFI() {}
