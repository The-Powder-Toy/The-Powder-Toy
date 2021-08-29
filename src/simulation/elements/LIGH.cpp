#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);
static void create_line_par(Simulation * sim, int x1, int y1, int x2, int y2, int c, float temp, int life, int tmp, int tmp2, int i);

void Element::Element_LIGH()
{
	Identifier = "DEFAULT_PT_LIGH";
	Name = "LIGH";
	Colour = PIXPACK(0xFFFFC0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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
	Hardness = 1;

	Weight = 100;

	HeatConduct = 0;
	Description = "Lightning. Change the brush size to set the size of the lightning.";

	Properties = TYPE_SOLID;

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
	Create = &create;
}

constexpr float LIGHTING_POWER = 0.65f;

static int update(UPDATE_FUNC_ARGS)
{
	/*
	 * tmp2:
	 * 0 - bending
	 * 1 - bending (particle order deferred)
	 * 2 - branching
	 * 3 - branching (particle order deferred)
	 * 4 - first pixel
	 * 5+  normal segment. Starts at 8, counts down and is removed at 5
	 *
	 * life - power of lightning, influences reaction strength and segment length
	 *
	 * tmp - angle of lighting, measured in degrees counterclockwise from the positive x direction
	 */
	int r,rx,ry,rt, multipler, powderful;
	float angle, angle2=-1;
	powderful = int(parts[i].temp*(1+parts[i].life/40)*LIGHTING_POWER);
	//Element_FIRE::update(UPDATE_FUNC_SUBCALL_ARGS);
	if (sim->aheat_enable)
	{
		sim->hv[y/CELL][x/CELL] += powderful/50;
		if (sim->hv[y/CELL][x/CELL] > MAX_TEMP)
			sim->hv[y/CELL][x/CELL] = MAX_TEMP;
		// If the LIGH was so powerful that it overflowed hv, set to max temp
		else if (sim->hv[y/CELL][x/CELL] < 0)
			sim->hv[y/CELL][x/CELL] = MAX_TEMP;
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = TYP(r);
				if ((surround_space || sim->elements[rt].Explosive) &&
				    (rt!=PT_SPNG || parts[ID(r)].life==0) &&
					sim->elements[rt].Flammable && RNG::Ref().chance(sim->elements[rt].Flammable + int(sim->pv[(y+ry)/CELL][(x+rx)/CELL] * 10.0f), 1000))
				{
					sim->part_change_type(ID(r),x+rx,y+ry,PT_FIRE);
					parts[ID(r)].temp = restrict_flt(sim->elements[PT_FIRE].DefaultProperties.temp + (sim->elements[rt].Flammable/2), MIN_TEMP, MAX_TEMP);
					parts[ID(r)].life = RNG::Ref().between(180, 259);
					parts[ID(r)].tmp = parts[ID(r)].ctype = 0;
					if (sim->elements[rt].Explosive)
						sim->pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
				switch (rt)
				{
				case PT_LIGH:
				case PT_TESC:
					continue;
				case PT_CLNE:
				case PT_THDR:
				case PT_DMND:
				case PT_FIRE:
					parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					continue;
				case PT_DEUT:
				case PT_PLUT:
					parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp+powderful, MIN_TEMP, MAX_TEMP);
					sim->pv[y/CELL][x/CELL] +=powderful/35;
					if (RNG::Ref().chance(1, 3))
					{
						sim->part_change_type(ID(r),x+rx,y+ry,PT_NEUT);
						parts[ID(r)].life = RNG::Ref().between(480, 959);
						parts[ID(r)].vx = float(RNG::Ref().between(-5, 5));
						parts[ID(r)].vy = float(RNG::Ref().between(-5, 5));
					}
					break;
				case PT_COAL:
				case PT_BCOL:
					if (parts[ID(r)].life>100)
						parts[ID(r)].life = 99;
					break;
				case PT_STKM:
					if (sim->player.elem!=PT_LIGH)
						parts[ID(r)].life-=powderful/100;
					break;
				case PT_STKM2:
					if (sim->player2.elem!=PT_LIGH)
						parts[ID(r)].life-=powderful/100;
					break;
				case PT_HEAC:
					parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					if (parts[ID(r)].temp > sim->elements[PT_HEAC].HighTemperature)
					{
						sim->part_change_type(ID(r), x+rx, y+ry, PT_LAVA);
						parts[ID(r)].ctype = PT_HEAC;
					}
					break;
				default:
					break;
				}
				if ((sim->elements[TYP(r)].Properties&PROP_CONDUCTS) && parts[ID(r)].life==0)
					sim->create_part(ID(r),x+rx,y+ry,PT_SPRK);
				sim->pv[y/CELL][x/CELL] += powderful/400;
				if (sim->elements[TYP(r)].HeatConduct) parts[ID(r)].temp = restrict_flt(parts[ID(r)].temp+powderful/1.3, MIN_TEMP, MAX_TEMP);
			}
	// Deferred branch or bend; or in removal countdown stage
	if (parts[i].tmp2 == 1 || parts[i].tmp2 == 3 || (parts[i].tmp2 >= 6 && parts[i].tmp2 <= 8))
	{
		// Probably set via console, make sure it doesn't stick around forever
		if (parts[i].tmp2 >= 9)
			parts[i].tmp2 = 7;
		else
			parts[i].tmp2--;
		return 0;
	}
	if (parts[i].tmp2 == 5 || parts[i].life <= 1)
	{
		sim->kill_part(i);
		return 1;
	}
	angle = float((parts[i].tmp + RNG::Ref().between(-30, 30)) % 360);
	multipler = int(parts[i].life * 1.5) + RNG::Ref().between(0, parts[i].life);
	rx=int(cos(angle*M_PI/180)*multipler);
	ry=int(-sin(angle*M_PI/180)*multipler);
	create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, int(angle), parts[i].tmp2, i);
	if (parts[i].tmp2 == 2)// && pNear == -1)
	{
		angle2 = float(((int)angle + RNG::Ref().between(-100, 100)) % 360);
		rx=int(cos(angle2*M_PI/180)*multipler);
		ry=int(-sin(angle2*M_PI/180)*multipler);
		create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, int(angle2), parts[i].tmp2, i);
	}

	parts[i].tmp2 = 7;
	return 0;
}

static bool create_LIGH(Simulation * sim, int x, int y, int c, float temp, int life, int tmp, int tmp2, bool last, int i)
{
	int p = sim->create_part(-1, x, y,c);
	if (p != -1)
	{
		sim->parts[p].temp = float(temp);
		sim->parts[p].tmp = tmp;
		if (last)
		{
			int nextSegmentLife = (int)(life/1.5 - RNG::Ref().between(0, 1));
			sim->parts[p].life = nextSegmentLife;
			if (nextSegmentLife > 1)
			{
				// Decide whether to branch or to bend
				bool doBranch = RNG::Ref().chance(7, 10);
				sim->parts[p].tmp2 = (doBranch ? 2 : 0) + (p > i && tmp2 != 4 ? 1 : 0);
			}
			// Not enough energy to continue
			else
			{
				sim->parts[p].tmp2 = 7 + (p > i ? 1 : 0);
			}
		}
		else
		{
			sim->parts[p].life = life;
			sim->parts[p].tmp2 = 7 + (p > i ? 1 : 0);
		}
	}
	else if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		int r = sim->pmap[y][x];
		if (((TYP(r)==PT_VOID || (TYP(r)==PT_PVOD && sim->parts[ID(r)].life >= 10)) && (!sim->parts[ID(r)].ctype || (sim->parts[ID(r)].ctype==c)!=(sim->parts[ID(r)].tmp&1))) || TYP(r)==PT_BHOL || TYP(r)==PT_NBHL) // VOID, PVOD, VACU, and BHOL eat LIGH here
			return true;
	}
	else return true;
	return false;
}

static void create_line_par(Simulation * sim, int x1, int y1, int x2, int y2, int c, float temp, int life, int tmp, int tmp2, int i)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1), back = false;
	int x, y, dx, dy, Ystep;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
		back = 1;
	dx = x2 - x1;
	dy = abs(y2 - y1);
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	Ystep = (y1<y2) ? 1 : -1;
	if (!back)
	{
		for (x = x1; x <= x2; x++)
		{
			bool ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2,x==x2, i);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2,x==x2, i);
			if (ret)
				return;

			e += de;
			if (e >= 0.5f)
			{
				y += Ystep;
				e -= 1.0f;
			}
		}
	}
	else
	{
		for (x = x1; x >= x2; x--)
		{
			bool ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2,x==x2, i);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2,x==x2, i);
			if (ret)
				return;

			e += de;
			if (e <= -0.5f)
			{
				y += Ystep;
				e += 1.0f;
			}
		}
	}
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 120;
	*firer = *colr = 235;
	*fireg = *colg = 245;
	*fireb = *colb = 255;
	*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	return 1;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	float gx, gy, gsize;
	if (v >= 0)
	{
		if (v > 55)
			v = 55;
		sim->parts[i].life = v;
	}
	else
		sim->parts[i].life = 30;
	sim->parts[i].temp = sim->parts[i].life * 150.0f; // temperature of the lightning shows the power of the lightning
	sim->GetGravityField(x, y, 1.0f, 1.0f, gx, gy);
	gsize = gx * gx + gy * gy;
	if (gsize < 0.0016f)
	{
		float angle = RNG::Ref().between(0, 6283) * 0.001f; //(in radians, between 0 and 2*pi)
		gsize = sqrtf(gsize);
		// randomness in weak gravity fields (more randomness with weaker fields)
		gx += cosf(angle) * (0.04f - gsize);
		gy += sinf(angle) * (0.04f - gsize);
	}
	sim->parts[i].tmp = (static_cast<int>(atan2f(-gy, gx) * (180.0f / M_PI)) + RNG::Ref().between(-20, 20) + 360) % 360;
	sim->parts[i].tmp2 = 4;
}
