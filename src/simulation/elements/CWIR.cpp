#include "simulation/ElementCommon.h"
#include "simulation/Air.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CWIR()
{
	Identifier = "DEFAULT_PT_CWIR";
	Name = "CWIR";
	Colour = PIXPACK(0xCC0000);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Meltable = 1;
	Hardness = 0;
	PhotonReflectWavelengths = 0xCC0000;

	Weight = 100;

	HeatConduct = 251;
	Description = "Customisable wire, conducts at a variable speed set by.tmp";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_HOT_GLOW | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1337.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	int rx, ry, r;
	int cust = parts[i].tmp;
	if (parts[i].tmp < 0 || parts[i].tmp > 30)
	{
		parts[i].tmp = 4;
	}

	int checkCoordsX[] = { -cust, cust, 0, 0 };
	int checkCoordsY[] = { 0, 0, -cust, cust };

	if (!parts[i].life)
	{
		for (int j = 0; j < 4; j++) {
			rx = checkCoordsX[j];
			ry = checkCoordsY[j];
			if (BOUNDS_CHECK) {
				r = pmap[y + ry][x + rx];
				if (!r) continue;
				if (TYP(r) == PT_SPRK && parts[ID(r)].life && parts[ID(r)].life < 4)
				{
					sim->part_change_type(i, x, y, PT_SPRK);
					parts[i].life = 4;
					parts[i].ctype = PT_CWIR;
				}
			}
		}
	}
	return 0;
}