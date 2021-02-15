#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_DMG()
{
	Identifier = "DEFAULT_PT_DMG";
	Name = "DMG";
	Colour = PIXPACK(0x88FF88);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.98f;
	Loss = 0.95f;
	Collision = 0.0f;
	Gravity = 0.1f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 20;

	Weight = 30;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Generates damaging pressure and breaks any elements it hits.";

	Properties = TYPE_PART|PROP_SPARKSETTLE;

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
	int r, rr, rx, ry, nxi, nxj, t, dist;
	int rad = 25;
	float angle, fx, fy;

	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_DMG && TYP(r)!=PT_EMBR && TYP(r)!=PT_DMND && TYP(r)!=PT_CLNE && TYP(r)!=PT_PCLN && TYP(r)!=PT_BCLN)
				{
					sim->kill_part(i);
					for (nxj=-rad; nxj<=rad; nxj++)
						for (nxi=-rad; nxi<=rad; nxi++)
							if (x+nxi>=0 && y+nxj>=0 && x+nxi<XRES && y+nxj<YRES && (nxi || nxj))
							{
								dist = int(sqrt(pow(nxi, 2.0f)+pow(nxj, 2.0f)));//;(pow((float)nxi,2))/(pow((float)rad,2))+(pow((float)nxj,2))/(pow((float)rad,2));
								if (!dist || (dist <= rad))
								{
									rr = pmap[y+nxj][x+nxi];
									if (rr)
									{
										angle = atan2((float)nxj, nxi);
										fx = cos(angle) * 7.0f;
										fy = sin(angle) * 7.0f;
										parts[ID(rr)].vx += fx;
										parts[ID(rr)].vy += fy;
										sim->vx[(y+nxj)/CELL][(x+nxi)/CELL] += fx;
										sim->vy[(y+nxj)/CELL][(x+nxi)/CELL] += fy;
										sim->pv[(y+nxj)/CELL][(x+nxi)/CELL] += 1.0f;
										t = TYP(rr);
										if (t && sim->elements[t].HighPressureTransition>-1 && sim->elements[t].HighPressureTransition<PT_NUM)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, sim->elements[t].HighPressureTransition);
										else if (t == PT_BMTL)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BRMT);
										else if (t == PT_GLAS)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BGLA);
										else if (t == PT_COAL)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BCOL);
										else if (t == PT_QRTZ)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_PQRT);
										else if (t == PT_TUNG)
										{
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_BRMT);
											parts[ID(rr)].ctype = PT_TUNG;
										}
										else if (t == PT_WOOD)
											sim->part_change_type(ID(rr), x+nxi, y+nxj, PT_SAWD);
									}
								}
							}
					return 1;
				}
			}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_FLARE;
	return 1;
}
