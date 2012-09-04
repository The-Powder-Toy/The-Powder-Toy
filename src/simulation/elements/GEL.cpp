#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_GEL PT_GEL 142
Element_GEL::Element_GEL()
{
    Identifier = "DEFAULT_PT_GEL";
    Name = "GEL";
    Colour = PIXPACK(0xFF9900);
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
    HotAir = 0.000f  * CFDS;
    Falldown = 2;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 20;
    
    Weight = 35;
    
    Temperature = R_TEMP-2.0f  +273.15f;
    HeatConduct = 29;
    Description = "Gel. A liquid with variable viscosity and heat conductivity";
    
    State = ST_LIQUID;
    Properties = TYPE_LIQUID|PROP_LIFE_DEC|PROP_NEUTPENETRATE;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_GEL::update;
    Graphics = &Element_GEL::graphics;
}

//#TPT-Directive ElementHeader Element_GEL static int update(UPDATE_FUNC_ARGS)
int Element_GEL::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	int absorbChanceDenom;
	if (parts[i].tmp>100) parts[i].tmp = 100;
	if (parts[i].tmp<0) parts[i].tmp = 0;
	absorbChanceDenom = parts[i].tmp*10 + 500;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				//Desaturation
				if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW || (r&0xFF)==PT_FRZW) && parts[i].tmp<100 && 500>rand()%absorbChanceDenom)
				{
					parts[i].tmp++;
					sim->kill_part(r>>8);
				}
				if (((r&0xFF)==PT_PSTE) && parts[i].tmp<100 && 20>rand()%absorbChanceDenom)
				{
					parts[i].tmp++;
					sim->create_part(r>>8, x+rx, y+ry, PT_CLST);
				}
				if (((r&0xFF)==PT_SLTW) && parts[i].tmp<100 && 50>rand()%absorbChanceDenom)
				{
					parts[i].tmp++;
					if (rand()%4)
						sim->kill_part(r>>8);
					else
						sim->part_change_type(r>>8, x+rx, y+ry, PT_SALT);
				}
				if (((r&0xFF)==PT_CBNW) && parts[i].tmp<100 && 100>rand()%absorbChanceDenom)
				{
					parts[i].tmp++;
					sim->part_change_type(r>>8, x+rx, y+ry, PT_CO2);
				}

				if ((r&0xFF)==PT_SPNG && parts[i].tmp<100 && ((parts[r>>8].life+1)>parts[i].tmp))
				{
					parts[r>>8].life--;
					parts[i].tmp++;
				}

				char gel = 0;
				if ((r&0xFF)==PT_GEL)
					gel = 1;

				//Concentration diffusion
				if (gel && (parts[r>>8].tmp+1)<parts[i].tmp)
				{
					parts[r>>8].tmp++;
					parts[i].tmp--;
				}

				if ((r&0xFF)==PT_SPNG && (parts[r>>8].life+1)<parts[i].tmp)
				{
					parts[r>>8].life++;
					parts[i].tmp--;
				}

				float dx, dy;
				dx = parts[i].x - parts[r>>8].x;
				dy = parts[i].y - parts[r>>8].y;

				//Stickness
				if ((dx*dx + dy*dy)>1.5 && (gel || !sim->elements[r&0xFF].Falldown || (fabs((float)rx)<2 && fabs((float)ry)<2)))
				{
					float per, nd;
					nd = dx*dx + dy*dy - 0.5;

					per = 5*(1 - parts[i].tmp/100)*(nd/(dx*dx + dy*dy + nd) - 0.5);
					if (sim->elements[r&0xFF].State==ST_LIQUID)
						per *= 0.1;

					dx *= per; dy *= per;
					parts[i].vx += dx;
					parts[i].vy += dy;
					if ((sim->elements[r&0xFF].Properties&TYPE_PART) || (r&0xFF)==PT_GOO)
					{
						parts[r>>8].vx -= dx;
						parts[r>>8].vy -= dy;
					}
				}
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_GEL static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GEL::graphics(GRAPHICS_FUNC_ARGS)

{
	int q = cpart->tmp;
	*colr = q*(32-255)/120+255;
	*colg = q*(48-186)/120+186;
	*colb = q*208/120;
	return 0;
}



Element_GEL::~Element_GEL() {}
