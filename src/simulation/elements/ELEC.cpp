#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ELEC PT_ELEC 136
Element_ELEC::Element_ELEC()
{
	Identifier = "DEFAULT_PT_ELEC";
	Name = "ELEC";
	Colour = PIXPACK(0xDFEFFF);
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	Temperature = R_TEMP+200.0f+273.15f;
	HeatConduct = 251;
	Description = "Electrons. Sparks electronics, reacts with NEUT and WATR.";

	Properties = TYPE_ENERGY|PROP_LIFE_DEC|PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_ELEC::update;
	Graphics = &Element_ELEC::graphics;
}

//#TPT-Directive ElementHeader Element_ELEC static int update(UPDATE_FUNC_ARGS)
int Element_ELEC::update(UPDATE_FUNC_ARGS)
{
	int r, rt, rx, ry, nb, rrx, rry;
	for (rx=-2; rx<=2; rx++)
		for (ry=-2; ry<=2; ry++)
			if (BOUNDS_CHECK) {
				r = pmap[y+ry][x+rx];
				if (!r)
					r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				switch (rt)
				{
				case PT_GLAS:
					for (rrx=-1; rrx<=1; rrx++)
						for (rry=-1; rry<=1; rry++)
							if (x+rx+rrx>=0 && y+ry+rry>=0 && x+rx+rrx<XRES && y+ry+rry<YRES) {
								nb = sim->create_part(-1, x+rx+rrx, y+ry+rry, PT_EMBR);
								if (nb!=-1) {
									parts[nb].tmp = 0;
									parts[nb].life = 50;
									parts[nb].temp = parts[i].temp*0.8f;
									parts[nb].vx = rand()%20-10;
									parts[nb].vy = rand()%20-10;
								}
							}
					sim->kill_part(i);
					return 1;
				case PT_LCRY:
					parts[r>>8].tmp2 = 5+rand()%5;
					break;
				case PT_WATR:
				case PT_DSTW:
				case PT_SLTW:
				case PT_CBNW:
					if(!(rand()%3))
						sim->create_part(r>>8, x+rx, y+ry, PT_O2);
					else
						sim->create_part(r>>8, x+rx, y+ry, PT_H2);
					sim->kill_part(i);
					return 1;
				case PT_PROT: // this is the correct reaction, not NEUT, but leaving NEUT in anyway
					if (parts[r>>8].tmp2 & 0x1)
						break;
				case PT_NEUT:
					sim->part_change_type(r>>8, x+rx, y+ry, PT_H2);
					parts[r>>8].life = 0;
					parts[r>>8].ctype = 0;
					sim->kill_part(i);
					break;
				case PT_DEUT:
					if(parts[r>>8].life < 6000)
						parts[r>>8].life += 1;
					parts[r>>8].temp = 0;
					sim->kill_part(i);
					return 1;
				case PT_EXOT:
					parts[r>>8].tmp2 += 5;
					parts[r>>8].life = 1000;
					break;
				case PT_NONE: //seems to speed up ELEC even if it isn't used
					break;
				default:
					if ((sim->elements[rt].Properties & PROP_CONDUCTS) && (rt!=PT_NBLE||parts[i].temp<2273.15))
					{
						sim->create_part(-1, x+rx, y+ry, PT_SPRK);
						sim->kill_part(i);
						return 1;
					}
					break;
				}
			}
	return 0;
}

//#TPT-Directive ElementHeader Element_ELEC static int graphics(GRAPHICS_FUNC_ARGS)
int Element_ELEC::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 70;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode |= FIRE_ADD;
	return 0;
}


Element_ELEC::~Element_ELEC() {}
