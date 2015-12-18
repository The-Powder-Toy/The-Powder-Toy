#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_WARP PT_WARP 96
Element_WARP::Element_WARP()
{
	Identifier = "DEFAULT_PT_WARP";
	Name = "WARP";
	Colour = PIXPACK(0x101010);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.8f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.9f;
	Loss = 0.70f;
	Collision = -0.1f;
	Gravity = 0.0f;
	Diffusion = 3.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 1;

	Temperature = R_TEMP +273.15f;
	HeatConduct = 100;
	Description = "Displaces other elements.";

	Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_WARP::update;
	Graphics = &Element_WARP::graphics;
}

//#TPT-Directive ElementHeader Element_WARP static int update(UPDATE_FUNC_ARGS)
int Element_WARP::update(UPDATE_FUNC_ARGS)
{
	int trade, r, rx, ry;
	if (parts[i].tmp2>2000)
	{
		parts[i].temp = 10000;
		sim->pv[y/CELL][x/CELL] += (parts[i].tmp2/5000) * CFDS;
		if (!(rand()%50))
			sim->create_part(-3, x, y, PT_ELEC);
	}
	for ( trade = 0; trade<5; trade ++)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if ((r&0xFF)!=PT_WARP&&(r&0xFF)!=PT_STKM&&(r&0xFF)!=PT_STKM2&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_BCLN&&(r&0xFF)!=PT_PCLN)
			{
				parts[i].x = parts[r>>8].x;
				parts[i].y = parts[r>>8].y;
				parts[r>>8].x = x;
				parts[r>>8].y = y;
				parts[r>>8].vx = (rand()%4)-1.5;
				parts[r>>8].vy = (rand()%4)-2;
				parts[i].life += 4;
				pmap[y][x] = r;
				pmap[y+ry][x+rx] = (i<<8)|parts[i].type;
				trade = 5;
			}
		}
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_WARP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_WARP::graphics(GRAPHICS_FUNC_ARGS)
{
	*colr = *colg = *colb = *cola = 0;
	*pixel_mode &= ~PMODE;
	return 0;
}

Element_WARP::~Element_WARP() {}
