#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_COAL PT_COAL 59
Element_COAL::Element_COAL()
{
	Identifier = "DEFAULT_PT_COAL";
	Name = "COAL";
	Colour = PIXPACK(0x222222);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.0f;
	HotAir = 0.0f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;
	PhotonReflectWavelengths = 0x00000000;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 200;
	Description = "Coal, Burns very slowly. Gets red when hot.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_COAL::update;
	Graphics = &Element_COAL::graphics;
}

//#TPT-Directive ElementHeader Element_COAL static int update(UPDATE_FUNC_ARGS)
int Element_COAL::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life<=0) {
		sim->create_part(i, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
	}
	if (parts[i].type == PT_COAL)
	{
		if ((sim->pv[y/CELL][x/CELL] > 4.3f)&&parts[i].tmp>40)
			parts[i].tmp=39;
		else if (parts[i].tmp<40&&parts[i].tmp>0)
			parts[i].tmp--;
		else if (parts[i].tmp<=0) {
			sim->create_part(i, x, y, PT_BCOL);
			return 1;
		}
	}
	if(parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = parts[i].temp;
	return 0;
}

#define FREQUENCY 3.1415/(2*300.0f-(300.0f-200.0f))

//#TPT-Directive ElementHeader Element_COAL static int graphics(GRAPHICS_FUNC_ARGS)
int Element_COAL::graphics(GRAPHICS_FUNC_ARGS)
 //Both COAL and Broken Coal
{
	*colr += (cpart->tmp2-295.15f)/3;

	if (*colr > 170)
		*colr = 170;
	if (*colr < *colg)
		*colr = *colg;
		
	*colg = *colb = *colr;

	// ((cpart->temp-295.15f) > 300.0f-200.0f)
	if (cpart->temp > 395.15f)
	{
		//  q = ((cpart->temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(cpart->temp-295.15f)-(300.0f-200.0f);
		int q = (cpart->temp > 595.15f) ? 200.0f : cpart->temp - 395.15f;

		*colr += sin(FREQUENCY*q) * 226;
		*colg += sin(FREQUENCY*q*4.55 + 3.14) * 34;
		*colb += sin(FREQUENCY*q*2.22 + 3.14) * 64;
	}
	return 0;
}



Element_COAL::~Element_COAL() {}
