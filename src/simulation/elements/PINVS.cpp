#include "simulation/Elements.h"
/*
  Created by TheChosenEvilOne, Feel free to remove this comment
*/
//#TPT-Directive ElementClass Element_PINVS PT_PINVS 183
Element_PINVS::Element_PINVS()
{
	Identifier = "DEFAULT_PT_PINVS";
	Name = "PNVS";
	Colour = PIXPACK(0x0FFFFF);
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
	Hardness = 15;

	Weight = 100;

	Temperature = R_TEMP+0.0f+273.15f;
	HeatConduct = 164;
	Description = "Powered Invisible, Invisible to particles when activated with PSCN.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PINVS::update;
	Graphics = &Element_PINVS::graphics;
}

//#TPT-Directive ElementHeader Element_PINVS static int update(UPDATE_FUNC_ARGS)
int Element_PINVS::update(UPDATE_FUNC_ARGS)
{
	int ry,rx,r;
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
					if ((r&0xFF)==PT_PINVS)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
}

//#TPT-Directive ElementHeader Element_PINVS static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PINVS::graphics(GRAPHICS_FUNC_ARGS)
{		

	// Modify graphics if you don't like them :)

	if(cpart->life == 10)
	{
		*cola = 100;
		*colr = 45;
		*colb = 180;
		*colg = 30;
		*pixel_mode = PMODE_BLEND;
	}
	return 0;
}


Element_PINVS::~Element_PINVS() {}
