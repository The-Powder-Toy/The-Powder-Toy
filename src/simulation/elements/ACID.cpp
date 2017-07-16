#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ACID PT_ACID 21
Element_ACID::Element_ACID()
{
	Identifier = "DEFAULT_PT_ACID";
	Name = "ACID";
	Colour = PIXPACK(0xED55FF);
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 40;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;
	PhotonReflectWavelengths = 0x1FE001FE;

	Weight = 10;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 34;
	Description = "Dissolves almost everything.";

	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_ACID::update;
	Graphics = &Element_ACID::graphics;
}

//#TPT-Directive ElementHeader Element_ACID static int update(UPDATE_FUNC_ARGS)
int Element_ACID::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, trade;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_ACID && (r&0xFF)!=PT_CAUS)
				{
					if ((r&0xFF)==PT_PLEX || (r&0xFF)==PT_NITR || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD)
					{
						sim->part_change_type(i,x,y,PT_FIRE);
						sim->part_change_type(r>>8,x+rx,y+ry,PT_FIRE);
						parts[i].life = 4;
						parts[r>>8].life = 4;
					}
					else if ((r&0xFF)==PT_WTRV)
					{
						if(!(rand()%250))
						{
							sim->part_change_type(i, x, y, PT_CAUS);
							parts[i].life = (rand()%50)+25;
							sim->kill_part(r>>8);
						}
					}
					else if (((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && sim->elements[r&0xFF].Hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (sim->parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
							float newtemp = ((60.0f-(float)sim->elements[r&0xFF].Hardness))*7.0f;
							if(newtemp < 0){
								newtemp = 0;
							}
							parts[i].temp += newtemp;
							parts[i].life--;
							sim->kill_part(r>>8);
						}
					}
					else if (parts[i].life<=50)
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
	for (trade = 0; trade<2; trade++)
	{
		rx = rand()%5-2;
		ry = rand()%5-2;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)==PT_ACID && (parts[i].life>parts[r>>8].life) && parts[i].life>0)//diffusion
			{
				int temp = parts[i].life - parts[r>>8].life;
				if (temp==1)
				{
					parts[r>>8].life++;
					parts[i].life--;
				}
				else if (temp>0)
				{
					parts[r>>8].life += temp/2;
					parts[i].life -= temp/2;
				}
			}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_ACID static int graphics(GRAPHICS_FUNC_ARGS)
int Element_ACID::graphics(GRAPHICS_FUNC_ARGS)
{
	int s = cpart->life;
	if (s>75) s = 75; //These two should not be here.
	if (s<49) s = 49;
	s = (s-49)*3;
	if (s==0) s = 1;
	*colr += s*4;
	*colg += s*1;
	*colb += s*2;
	*pixel_mode |= PMODE_BLUR;
	return 0;
}

Element_ACID::~Element_ACID() {}
