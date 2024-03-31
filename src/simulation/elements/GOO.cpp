#include "simulation/ElementCommon.h"

#include "audio/AudioEngine.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_GOO()
{
	Identifier = "DEFAULT_PT_GOO";
	Name = "SQRL";
	Colour = 0x804000_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.50f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 12;
	PhotonReflectWavelengths = 0x3FFAAA00;

	Weight = 100;

	HeatConduct = 75;
	Description = "Squirrels. \"Every time we cut an emoji for a squirrel we have to cut it from tpt as well\" - jacob1";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
}

constexpr float ADVECTION = 0.1f;

static int update(UPDATE_FUNC_ARGS)
{
	if (sim->rng.chance(1, 2400))
	{
		sim->ae->Play(194);
	}
	if (!parts[i].life && sim->pv[y/CELL][x/CELL]>1.0f)
		parts[i].life = sim->rng.between(300, 379);
	if (parts[i].life)
	{
		parts[i].vx += ADVECTION*sim->vx[y/CELL][x/CELL];
		parts[i].vy += ADVECTION*sim->vy[y/CELL][x/CELL];
	}
	return 0;
}
