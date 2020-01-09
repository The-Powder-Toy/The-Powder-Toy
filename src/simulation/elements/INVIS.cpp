#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_INVIS()
{
	Identifier = "DEFAULT_PT_INVIS";
	Name = "INVS";
	Colour = PIXPACK(0x00CCCC);
	MenuVisible = 1;
	MenuSection = SC_SENSOR;
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
	Hardness = 15;

	Weight = 100;

	HeatConduct = 164;
	Description = "Invisible to particles while under pressure.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	float pressureResistance = 0.0f;
	if (parts[i].tmp > 0)
		pressureResistance = (float) parts[i].tmp;
	else
		pressureResistance = 4.0f;

	if (sim->pv[y/CELL][x/CELL] < -pressureResistance || sim->pv[y/CELL][x/CELL] > pressureResistance)
		parts[i].tmp2 = 1;
	else
		parts[i].tmp2 = 0;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	//pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f
	if(cpart->tmp2)
	{
		*cola = 100;
		*colr = 15;
		*colg = 0;
		*colb = 150;
		*pixel_mode = PMODE_BLEND;
	}
	return 0;
}
