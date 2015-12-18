#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WIRE PT_WIRE 156
Element_WIRE::Element_WIRE()
{
	Identifier = "DEFAULT_PT_WIRE";
	Name = "WWLD";
	Colour = PIXPACK(0xFFCC00);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 250;
	Description = "WireWorld wires, conducts based on a set of GOL-like rules.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_WIRE::update;
	Graphics = &Element_WIRE::graphics;
}

//#TPT-Directive ElementHeader Element_WIRE static int update(UPDATE_FUNC_ARGS)
int Element_WIRE::update(UPDATE_FUNC_ARGS)
{
	int r,rx,ry,count=0;
	/*
	  0:  wire
	  1:  spark head
	  2:  spark tail
	  
	  tmp is previous state, ctype is current state
	*/
	//parts[i].tmp=parts[i].ctype;
	parts[i].ctype=0;
	if (parts[i].tmp==1)
	{
		parts[i].ctype=2;
	}
	else if (parts[i].tmp==2)
	{
		parts[i].ctype=0;
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3 && parts[r>>8].ctype==PT_PSCN)
				{
					parts[i].ctype=1;
					return 0;
				}
				else if ((r&0xFF)==PT_NSCN && parts[i].tmp==1)
					sim->create_part(-1, x+rx, y+ry, PT_SPRK);
				else if ((r&0xFF)==PT_WIRE && parts[r>>8].tmp==1 && !parts[i].tmp)
					count++;
			}
		}
	if (count==1 || count==2)
		parts[i].ctype=1;
	return 0;
}



//#TPT-Directive ElementHeader Element_WIRE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_WIRE::graphics(GRAPHICS_FUNC_ARGS)

{
	if (cpart->ctype==0)
	{
		*colr = 255;
		*colg = 204;
		*colb = 0;
		return 0;
	}
	if (cpart->ctype==1)
	{
		*colr = 50;
		*colg = 100;
		*colb = 255;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
	if (cpart->ctype==2)
	{
		*colr = 255;
		*colg = 100;
		*colb = 50;
		//*pixel_mode |= PMODE_GLOW;
		return 0;
	}
	return 0;
}


Element_WIRE::~Element_WIRE() {}
