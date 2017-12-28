#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_EMBR PT_EMBR 147
Element_EMBR::Element_EMBR()
{
	Identifier = "DEFAULT_PT_EMBR";
	Name = "EMBR";
	Colour = PIXPACK(0xFFF288);
	MenuVisible = 0;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.001f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.90f;
	Collision = 0.0f;
	Gravity = 0.07f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	Temperature = 500.0f +273.15f;
	HeatConduct = 29;
	Description = "Sparks. Formed by explosions.";

	Properties = TYPE_PART|PROP_LIFE_DEC|PROP_LIFE_KILL|PROP_SPARKSETTLE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_EMBR::update;
	Graphics = &Element_EMBR::graphics;
}

//#TPT-Directive ElementHeader Element_EMBR static int update(UPDATE_FUNC_ARGS)
int Element_EMBR::update(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((sim->elements[TYP(r)].Properties & (TYPE_SOLID | TYPE_PART | TYPE_LIQUID)) && !(sim->elements[TYP(r)].Properties & PROP_SPARKSETTLE))
				{
					sim->kill_part(i);
					return 1;
				}
			}
	return 0;
}

//#TPT-Directive ElementHeader Element_EMBR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_EMBR::graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->ctype&0xFFFFFF)
	{
		int maxComponent;
		*colr = (cpart->ctype&0xFF0000)>>16;
		*colg = (cpart->ctype&0x00FF00)>>8;
		*colb = (cpart->ctype&0x0000FF);
		maxComponent = *colr;

		if (*colg>maxComponent) maxComponent = *colg;
		if (*colb>maxComponent) maxComponent = *colb;
		if (maxComponent<60)//make sure it isn't too dark to see
		{
			float multiplier = 60.0f/maxComponent;
			*colr *= multiplier;
			*colg *= multiplier;
			*colb *= multiplier;
		}
	}
	else if (cpart->tmp != 0)
	{
		*colr = *colg = *colb = 255;
	}

	if (ren->decorations_enable && cpart->dcolour)
	{
		int a = (cpart->dcolour>>24)&0xFF;
		*colr = (a*((cpart->dcolour>>16)&0xFF) + (255-a)**colr) >> 8;
		*colg = (a*((cpart->dcolour>>8)&0xFF) + (255-a)**colg) >> 8;
		*colb = (a*((cpart->dcolour)&0xFF) + (255-a)**colb) >> 8;
	}
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	if (cpart->tmp==1)
	{
		*pixel_mode = FIRE_ADD | PMODE_BLEND | PMODE_GLOW;
		*firea = (cpart->life-15)*4;
		*cola = (cpart->life+15)*4;
	}
	else if (cpart->tmp==2)
	{
		*pixel_mode = PMODE_FLAT | FIRE_ADD;
		*firea = 255;
	}
	else
	{
		*pixel_mode = PMODE_SPARK | PMODE_ADD;
		if (cpart->life<64) *cola = 4*cpart->life;
	}
	return 0;
}

Element_EMBR::~Element_EMBR() {}
