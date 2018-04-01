#include "common/tpt-minmax.h"
#include "simulation/Elements.h"

//#TPT-Directive ElementClass Element_GRAV PT_GRAV 102
Element_GRAV::Element_GRAV()
{
	Identifier = "DEFAULT_PT_GRAV";
	Name = "GRAV";
	Colour = PIXPACK(0x202020);
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 10;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 85;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 70;
	Description = "Very light dust. Changes colour based on velocity.";

	Properties = TYPE_PART | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_GRAV::update;
	Graphics = &Element_GRAV::graphics;
}

//#TPT-Directive ElementHeader Element_GRAV static int update(UPDATE_FUNC_ARGS)
int Element_GRAV::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].vx*parts[i].vx + parts[i].vy*parts[i].vy >= 0.1f && (rand() % 512) == 0)
	{
		if (!parts[i].life)
			parts[i].life = 48;
	}

	return 0;
}

//#TPT-Directive ElementHeader Element_GRAV static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GRAV::graphics(GRAPHICS_FUNC_ARGS)
{
	int GRAV_R, GRAV_B, GRAV_G, GRAV_R2, GRAV_B2, GRAV_G2;

	GRAV_R = std::abs((ren->sim->currentTick%120)-60);
	GRAV_G = std::abs(((ren->sim->currentTick+60)%120)-60);
	GRAV_B = std::abs(((ren->sim->currentTick+120)%120)-60);
	GRAV_R2 = std::abs((ren->sim->currentTick%60)-30);
	GRAV_G2 = std::abs(((ren->sim->currentTick+30)%60)-30);
	GRAV_B2 = std::abs(((ren->sim->currentTick+60)%60)-30);


	*colr = 20;
	*colg = 20;
	*colb = 20;
	if (cpart->vx>0)
	{
		*colr += (cpart->vx)*GRAV_R;
		*colg += (cpart->vx)*GRAV_G;
		*colb += (cpart->vx)*GRAV_B;
	}
	if (cpart->vy>0)
	{
		*colr += (cpart->vy)*GRAV_G;
		*colg += (cpart->vy)*GRAV_B;
		*colb += (cpart->vy)*GRAV_R;

	}
	if (cpart->vx<0)
	{
		*colr -= (cpart->vx)*GRAV_B;
		*colg -= (cpart->vx)*GRAV_R;
		*colb -= (cpart->vx)*GRAV_G;

	}
	if (cpart->vy<0)
	{
		*colr -= (cpart->vy)*GRAV_R2;
		*colg -= (cpart->vy)*GRAV_G2;
		*colb -= (cpart->vy)*GRAV_B2;
	}

	if (cpart->life)
	{
		*pixel_mode = FIRE_ADD | PMODE_ADD | PMODE_GLOW | PMODE_FLARE;
		*firer = std::min(*colr * 3, 255);
		*fireg = std::min(*colg * 3, 255);
		*fireb = std::min(*colb * 3, 255);
		*firea = (cpart->life+15)*4;
		*cola = (cpart->life+15)*4;
	}
	else
		*pixel_mode = PMODE_ADD;

	return 0;
}


Element_GRAV::~Element_GRAV() {}
