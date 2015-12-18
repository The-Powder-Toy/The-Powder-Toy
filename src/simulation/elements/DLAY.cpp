#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_DLAY PT_DLAY 79
Element_DLAY::Element_DLAY()
{
	Identifier = "DEFAULT_PT_DLAY";
	Name = "DLAY";
	Colour = PIXPACK(0x753590);
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
	Hardness = 1;

	Weight = 100;

	Temperature = 4.0f+273.15f;
	HeatConduct = 0;
	Description = "Conducts with temperature-dependent delay. (use HEAT/COOL).";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_DLAY::update;
	Graphics = &Element_DLAY::graphics;
}

//#TPT-Directive ElementHeader Element_DLAY static int update(UPDATE_FUNC_ARGS)
int Element_DLAY::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, oldl;
	oldl = parts[i].life;
	if (parts[i].life>0)
		parts[i].life--;
	if (parts[i].temp<= 1.0f+273.15f)
		parts[i].temp = 1.0f+273.15f;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r || sim->parts_avg(r>>8, i,PT_INSL)==PT_INSL)
					continue;
				if ((r&0xFF)==PT_SPRK && parts[i].life==0 && parts[r>>8].life>0 && parts[r>>8].life<4 && parts[r>>8].ctype==PT_PSCN)
				{
					parts[i].life = (int)(parts[i].temp-273.15f+0.5f);
				}
				else if ((r&0xFF)==PT_DLAY)
				{
					if (!parts[i].life)
					{
						if (parts[r>>8].life)
						{
							parts[i].life = parts[r>>8].life;
							if((r>>8)>i) //If the other particle hasn't been life updated
								parts[i].life--;
						}
					}
					else if (!parts[r>>8].life)
					{
						parts[r>>8].life = parts[i].life;
						if((r>>8)>i) //If the other particle hasn't been life updated
							parts[r>>8].life++;
					}
				}
				else if((r&0xFF)==PT_NSCN && oldl==1)
				{
					sim->create_part(-1, x+rx, y+ry, PT_SPRK);
				}
			}
	//}
	return 0;
}


//#TPT-Directive ElementHeader Element_DLAY static int graphics(GRAPHICS_FUNC_ARGS)
int Element_DLAY::graphics(GRAPHICS_FUNC_ARGS)

{
	int stage = (int)(((float)cpart->life/(cpart->temp-273.15))*100.0f);
	*colr += stage;
	*colg += stage;
	*colb += stage;
	return 0;
}


Element_DLAY::~Element_DLAY() {}
