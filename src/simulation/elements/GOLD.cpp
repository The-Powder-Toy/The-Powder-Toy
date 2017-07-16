#include "simulation/Elements.h"
#include "simulation/Air.h"
//#TPT-Directive ElementClass Element_GOLD PT_GOLD 170
Element_GOLD::Element_GOLD()
{
	Identifier = "DEFAULT_PT_GOLD";
	Name = "GOLD";
	Colour = PIXPACK(0xDCAD2C);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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
	PhotonReflectWavelengths = 0x3C038100;

	Weight = 100;

	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 251;
	Description = "Corrosion resistant metal, will reverse corrosion of iron.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_HOT_GLOW|PROP_LIFE_DEC|PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1337.0f;
	HighTemperatureTransition = PT_LAVA;

	Update = &Element_GOLD::update;
	Graphics = &Element_GOLD::graphics;
}

//#TPT-Directive ElementHeader Element_GOLD static int update(UPDATE_FUNC_ARGS)
int Element_GOLD::update(UPDATE_FUNC_ARGS)
{
	int rx, ry, r, rndstore;
	static int checkCoordsX[] = { -4, 4, 0, 0 };
	static int checkCoordsY[] = { 0, 0, -4, 4 };
	//Find nearby rusted iron (BMTL with tmp 1+)
	for(int j = 0; j < 8; j++){
		rndstore = rand();
		rx = (rndstore % 9)-4;
		rndstore >>= 4;
		ry = (rndstore % 9)-4;
		if ((!rx != !ry) && BOUNDS_CHECK) {
			r = pmap[y+ry][x+rx];
			if(!r) continue;
			if((r&0xFF)==PT_BMTL && parts[r>>8].tmp)
			{
				parts[r>>8].tmp = 0;
				sim->part_change_type(r>>8, x+rx, y+ry, PT_IRON);
			}
		}
	}
	//Find sparks
	if(!parts[i].life)
	{
		for(int j = 0; j < 4; j++){
			rx = checkCoordsX[j];
			ry = checkCoordsY[j];
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if(!r) continue;
				if((r&0xFF)==PT_SPRK && parts[r>>8].life && parts[r>>8].life<4)
				{
					sim->part_change_type(i, x, y, PT_SPRK);
					parts[i].life = 4;
					parts[i].ctype = PT_GOLD;
				}
			}
		}
	}
	if ((sim->photons[y][x]&0xFF) == PT_NEUT)
	{
		if (!(rand()%7))
		{
			sim->kill_part(sim->photons[y][x]>>8);
		}
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_GOLD static int graphics(GRAPHICS_FUNC_ARGS)
int Element_GOLD::graphics(GRAPHICS_FUNC_ARGS)
{
	int rndstore = rand();
	*colr += (rndstore % 10) - 5;
	rndstore >>= 4;
	*colg += (rndstore % 10)- 5;
	rndstore >>= 4;
	*colb += (rndstore % 10) - 5;
	return 0;
}

Element_GOLD::~Element_GOLD() {}
