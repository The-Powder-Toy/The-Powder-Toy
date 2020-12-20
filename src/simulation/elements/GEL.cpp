#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_GEL()
{
	Identifier = "DEFAULT_PT_GEL";
	Name = "GEL";
	Colour = PIXPACK(0xFF9900);
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
	HotAir = 0.000f  * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 35;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Gel. A liquid with variable viscosity and heat conductivity.";

	Properties = TYPE_LIQUID|PROP_LIFE_DEC|PROP_NEUTPENETRATE;

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
	int r, rx, ry, rt;
	bool gel;
	if (parts[i].tmp > 100)
		parts[i].tmp = 100;
	if (parts[i].tmp < 0)
		parts[i].tmp = 0;
	int absorbChanceDenom = parts[i].tmp * 10 + 500;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				gel=false;
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = TYP(r);
				//Desaturation
				switch (rt)
				{
				case PT_WATR:
				case PT_DSTW:
				case PT_FRZW:
					if (parts[i].tmp<100 && RNG::Ref().chance(500, absorbChanceDenom))
					{
						parts[i].tmp++;
						sim->kill_part(ID(r));
					}
					break;
				case PT_PSTE:
					if (parts[i].tmp<100 && RNG::Ref().chance(20, absorbChanceDenom))
					{
						parts[i].tmp++;
						sim->create_part(ID(r), x+rx, y+ry, PT_CLST);
					}
					break;
				case PT_SLTW:
					if (parts[i].tmp<100 && RNG::Ref().chance(50, absorbChanceDenom))
					{
						parts[i].tmp++;
						if (RNG::Ref().chance(3, 4))
							sim->kill_part(ID(r));
						else
							sim->part_change_type(ID(r), x+rx, y+ry, PT_SALT);
					}
					break;
				case PT_CBNW:
					if (parts[i].tmp < 100 && RNG::Ref().chance(100, absorbChanceDenom))
					{
						parts[i].tmp++;
						sim->part_change_type(ID(r), x+rx, y+ry, PT_CO2);
					}
					break;
				case PT_SPNG:
					// Concentration diffusion
					if (parts[ID(r)].life>0 && parts[i].tmp<100 && ((parts[ID(r)].life+1)>parts[i].tmp))
					{
						// SPNG -> GEL
						parts[ID(r)].life--;
						parts[i].tmp++;
					}
					else if (parts[i].tmp>0 && (parts[ID(r)].life+1)<parts[i].tmp)
					{
						// SPNG <- GEL (saturation limit of SPNG is ignored here)
						parts[ID(r)].life++;
						parts[i].tmp--;
					}
					break;
				case PT_GEL:
					if ((parts[ID(r)].tmp+1)<parts[i].tmp)
					{
						parts[ID(r)].tmp++;
						parts[i].tmp--;
					}
					gel = true;
					break;
				default:
					break;
				}
				float dx, dy;
				dx = parts[i].x - parts[ID(r)].x;
				dy = parts[i].y - parts[ID(r)].y;

				//Stickiness
				if ((dx*dx + dy*dy)>1.5 && (gel || !sim->elements[rt].Falldown || (fabs((float)rx)<2 && fabs((float)ry)<2)))
				{
					float per, nd;
					nd = dx*dx + dy*dy - 0.5;
					per = 5*(1 - parts[i].tmp/100)*(nd/(dx*dx + dy*dy + nd) - 0.5);
					if (sim->elements[rt].Properties&TYPE_LIQUID)
						per *= 0.1;
					dx *= per; dy *= per;
					parts[i].vx += dx;
					parts[i].vy += dy;
					if ((sim->elements[rt].Properties&TYPE_PART) || rt==PT_GOO)
					{
						parts[ID(r)].vx -= dx;
						parts[ID(r)].vy -= dy;
					}
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int q = cpart->tmp;
	*colr = q*(32-255)/120+255;
	*colg = q*(48-186)/120+186;
	*colb = q*208/120;
	*pixel_mode |= PMODE_BLUR;
	return 0;
}
