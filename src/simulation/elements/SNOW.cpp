#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SNOW PT_SNOW 16
Element_SNOW::Element_SNOW()
{
	Identifier = "DEFAULT_PT_SNOW";
	Name = "SNOW";
	Colour = PIXPACK(0xC0E0FF);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.90f;
	Collision = -0.1f;
	Gravity = 0.05f;
	Diffusion = 0.01f;
	HotAir = -0.00005f* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	PhotonReflectWavelengths = 0x03FFFFFF;

	Weight = 50;

	Temperature = R_TEMP-30.0f+273.15f;
	HeatConduct = 46;
	Description = "Light particles. Created when ICE breaks under pressure.";

	Properties = TYPE_PART|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 252.05f;
	HighTemperatureTransition = ST;

	Update = &Element_SNOW::update;
}

//#TPT-Directive ElementHeader Element_SNOW static int update(UPDATE_FUNC_ARGS)
int Element_SNOW::update(UPDATE_FUNC_ARGS)
 { //currently used for snow as well
	int r, rx, ry;
	if (parts[i].ctype==PT_FRZW)//get colder if it is from FRZW
	{
		parts[i].temp = restrict_flt(parts[i].temp-1.0f, MIN_TEMP, MAX_TEMP);
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((TYP(r)==PT_SALT || TYP(r)==PT_SLTW) && !(rand()%333))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SLTW);
				}
			}
	return 0;
}


Element_SNOW::~Element_SNOW() {}
