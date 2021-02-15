#include "simulation/ElementCommon.h"

int Element_BIZR_update(UPDATE_FUNC_ARGS);
int Element_BIZR_graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BIZR()
{
	Identifier = "DEFAULT_PT_BIZR";
	Name = "BIZR";
	Colour = PIXPACK(0x00FF77);
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
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	HeatConduct = 29;
	Description = "Bizarre... contradicts the normal state changes. Paints other elements with its deco color.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 100.0f;
	LowTemperatureTransition = PT_BIZRG;
	HighTemperature = 400.0f;
	HighTemperatureTransition = PT_BIZRS;

	DefaultProperties.ctype = 0x47FFFF;

	Update = &Element_BIZR_update;
	Graphics = &Element_BIZR_graphics;
}

constexpr float BLEND = 0.95f;

int Element_BIZR_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, nr, ng, nb, na;
	float tr, tg, tb, ta, mr, mg, mb, ma;
	if(parts[i].dcolour){
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)!=PT_BIZR && TYP(r)!=PT_BIZRG  && TYP(r)!=PT_BIZRS)
					{
						tr = float((parts[ID(r)].dcolour>>16)&0xFF);
						tg = float((parts[ID(r)].dcolour>>8)&0xFF);
						tb = float((parts[ID(r)].dcolour)&0xFF);
						ta = float((parts[ID(r)].dcolour>>24)&0xFF);

						mr = float((parts[i].dcolour>>16)&0xFF);
						mg = float((parts[i].dcolour>>8)&0xFF);
						mb = float((parts[i].dcolour)&0xFF);
						ma = float((parts[i].dcolour>>24)&0xFF);

						nr = int((tr*BLEND) + (mr*(1 - BLEND)));
						ng = int((tg*BLEND) + (mg*(1 - BLEND)));
						nb = int((tb*BLEND) + (mb*(1 - BLEND)));
						na = int((ta*BLEND) + (ma*(1 - BLEND)));

						parts[ID(r)].dcolour = nr<<16 | ng<<8 | nb | na<<24;
					}
				}
	}
	return 0;
}

int Element_BIZR_graphics(GRAPHICS_FUNC_ARGS)
 //BIZR, BIZRG, BIZRS
{
	int x = 0;
	float brightness = fabs(cpart->vx) + fabs(cpart->vy);
	if (cpart->ctype&0x3FFFFFFF)
	{
		*colg = 0;
		*colb = 0;
		*colr = 0;
		for (x=0; x<12; x++) {
			*colr += (cpart->ctype >> (x+18)) & 1;
			*colb += (cpart->ctype >>  x)     & 1;
		}
		for (x=0; x<12; x++)
			*colg += (cpart->ctype >> (x+9))  & 1;
		x = 624 / (*colr + *colg + *colb + 1);
		*colr *= x;
		*colg *= x;
		*colb *= x;
	}

	if(brightness>0)
	{
		brightness /= 5;
		*firea = 255;
		*fireg = int(*colg * brightness);
		*fireb = int(*colb * brightness);
		*firer = int(*colr * brightness);
		*pixel_mode |= FIRE_ADD;
	}
	*pixel_mode |= PMODE_BLUR;
	return 0;
}
