#include "simulation/ElementCommon.h"

int Element_COAL_update(UPDATE_FUNC_ARGS);
int Element_COAL_graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_COAL()
{
	Identifier = "DEFAULT_PT_COAL";
	Name = "COAL";
	Colour = 0x222222_rgb;
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

	DefaultProperties.life = 110;
	DefaultProperties.tmp = 50;

	Update = &Element_COAL_update;
	Graphics = &Element_COAL_graphics;
}

int Element_COAL_update(UPDATE_FUNC_ARGS)
{
	if (parts[i].life<=0) {
		sim->create_part(i, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_FIRE);
	}
	if (parts[i].type == PT_COAL)
	{
		if ((sim->pv[y/CELL][x/CELL] > 4.3f)&&parts[i].tmp>40)
			parts[i].tmp=39;
		else if (parts[i].tmp<40&&parts[i].tmp>0)
			parts[i].tmp--;
		else if (parts[i].tmp<=0) {
			sim->part_change_type(i, x, y, PT_BCOL);
			return 1;
		}
	}
	if(parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = int(parts[i].temp);
	return 0;
}

constexpr float FREQUENCY = 3.1415f/(2*300.0f-(300.0f-200.0f));

int Element_COAL_graphics(GRAPHICS_FUNC_ARGS)
 //Both COAL and Broken Coal
{
	*colr += int((cpart->tmp2-295.15f)/3);

	if (*colr > 170)
		*colr = 170;
	if (*colr < *colg)
		*colr = *colg;

	*colg = *colb = *colr;

	// ((cpart->temp-295.15f) > 300.0f-200.0f)
	if (cpart->temp > 395.15f)
	{
		//  q = ((cpart->temp-295.15f)>300.0f)?300.0f-(300.0f-200.0f):(cpart->temp-295.15f)-(300.0f-200.0f);
		auto q = int((cpart->temp > 595.15f) ? 200.0f : cpart->temp - 395.15f);

		*colr += int(sin(FREQUENCY*q) * 226);
		*colg += int(sin(FREQUENCY*q*4.55 + TPT_PI_DBL) * 34);
		*colb += int(sin(FREQUENCY*q*2.22 + TPT_PI_DBL) * 64);
	}
	return 0;
}
