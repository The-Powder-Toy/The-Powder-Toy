#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PLNT PT_PLNT 20
Element_PLNT::Element_PLNT()
{
    Identifier = "DEFAULT_PT_PLNT";
    Name = "PLNT";
    Colour = PIXPACK(0x0CAC00);
    MenuVisible = 1;
    MenuSection = SC_SOLIDS;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.95f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 20;
    Explosive = 0;
    Meltable = 0;
    Hardness = 10;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 65;
    Description = "Plant, drinks water and grows.";
    
    State = ST_SOLID;
    Properties = TYPE_SOLID|PROP_NEUTPENETRATE|PROP_LIFE_DEC;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 573.0f;
    HighTemperatureTransition = PT_FIRE;
    
    Update = &Element_PLNT::update;
    Graphics = &Element_PLNT::graphics;
}

//#TPT-Directive ElementHeader Element_PLNT static int update(UPDATE_FUNC_ARGS)
int Element_PLNT::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, np;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>(rand()%250))
				{
					np = sim->create_part(r>>8,x+rx,y+ry,PT_PLNT);
					if (np<0) continue;
					parts[np].life = 0;
				}
				else if ((r&0xFF)==PT_LAVA && 1>(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
				}
				else if (((r&0xFF)==PT_SMKE || (r&0xFF)==PT_CO2) && (1>rand()%250))
				{
					sim->kill_part(r>>8);
					parts[i].life = rand()%60 + 60;
				}
				else if (surround_space && ((r&0xFF)==PT_WOOD) && (1>rand()%20) && (abs(rx+ry)<=2) && (sim->VINE_MODE || parts[i].tmp==1) )  
				{
					int nnx = rand()%3 -1;
					int nny = rand()%3 -1;
					if (x+rx+nnx>=0 && y+ry+nny>0 && x+rx+nnx<XRES && y+ry+nny<YRES && (nnx || nny))
					{
						if (pmap[y+ry+nny][x+rx+nnx])
							continue;
						np = sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_VINE);
						if (np<0) continue;
						parts[np].temp = parts[i].temp;
					}
				}
			}
	if (parts[i].life==2)
	{
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						sim->create_part(-1,x+rx,y+ry,PT_O2);
				}
		parts[i].life = 0;
	}
	if (parts[i].temp > 400 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

//#TPT-Directive ElementHeader Element_PLNT static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PLNT::graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = fmax(cpart->tmp2, cpart->temp);
	if (maxtemp > 300)
	{
		*colr += (int)restrict_flt((maxtemp-300)/5,0,58);
		*colg -= (int)restrict_flt((maxtemp-300)/2,0,102);
		*colb += (int)restrict_flt((maxtemp-300)/5,0,70);
	}
	if (maxtemp < 273)
	{
		*colg += (int)restrict_flt((273-maxtemp)/4,0,255);
		*colb += (int)restrict_flt((273-maxtemp)/1.5,0,255);
	}
	return 0;
}


Element_PLNT::~Element_PLNT() {}