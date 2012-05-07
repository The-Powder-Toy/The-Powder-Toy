#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_O2 PT_O2 61
Element_O2::Element_O2()
{
    Identifier = "DEFAULT_PT_O2";
    Name = "OXYG";
    Colour = PIXPACK(0x80A0FF);
    MenuVisible = 1;
    MenuSection = SC_GAS;
    Enabled = 1;
    
    Advection = 2.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.99f;
    Loss = 0.30f;
    Collision = -0.1f;
    Gravity = 0.0f;
    Diffusion = 3.0f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 0;
    
    Weight = 1;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 70;
    Description = "Gas. Ignites easily.";
    
    State = ST_GAS;
    Properties = TYPE_GAS;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = 90.0f;
    LowTemperatureTransition = PT_LO2;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_O2::update;
    Graphics = NULL;
}

//#TPT-Directive ElementHeader Element_O2 static int update(UPDATE_FUNC_ARGS)
int Element_O2::update(UPDATE_FUNC_ARGS)

{
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				if ((r&0xFF)==PT_FIRE)
				{
					parts[r>>8].temp+=(rand()/(RAND_MAX/100));
					if(parts[r>>8].tmp&0x01)
						parts[r>>8].temp=3473;
					parts[r>>8].tmp |= 2;
				}
				if ((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM)
				{
					sim->create_part(i,x,y,PT_FIRE);
					parts[i].temp+=(rand()/(RAND_MAX/100));
					parts[i].tmp |= 2;
				}

			}
	return 0;
}


Element_O2::~Element_O2() {}