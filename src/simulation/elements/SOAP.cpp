#include "simulation/ElementCommon.h"
#include "SOAP.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS);

void Element::Element_SOAP()
{
	Identifier = "DEFAULT_PT_SOAP";
	Name = "SOAP";
	Colour = 0xF5F5DC_rgb;
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
	Hardness = 19;

	Weight = 35;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 29;
	Description = "Soap. Creates bubbles, washes off deco color, and cures virus.";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.tmp = -1;
	DefaultProperties.tmp2 = -1;

	Update = &update;
	Graphics = &graphics;
	ChangeType = &changeType;
}

static bool validIndex(int i)
{
	return i >= 0 && i < NPART;
}

void Element_SOAP_detach(Simulation * sim, int i)
{
	if ((sim->parts[i].ctype&2) == 2 && validIndex(sim->parts[i].tmp) && sim->parts[sim->parts[i].tmp].type == PT_SOAP)
	{
		if ((sim->parts[sim->parts[i].tmp].ctype&4) == 4)
			sim->parts[sim->parts[i].tmp].ctype ^= 4;
	}

	if ((sim->parts[i].ctype&4) == 4 && validIndex(sim->parts[i].tmp2) && sim->parts[sim->parts[i].tmp2].type == PT_SOAP)
	{
		if ((sim->parts[sim->parts[i].tmp2].ctype&2) == 2)
			sim->parts[sim->parts[i].tmp2].ctype ^= 2;
	}

	sim->parts[i].ctype = 0;
}

static void attach(Particle * parts, int i1, int i2)
{
	if (!(parts[i2].ctype&4))
	{
		parts[i1].ctype |= 2;
		parts[i1].tmp = i2;

		parts[i2].ctype |= 4;
		parts[i2].tmp2 = i1;
	}
	else if (!(parts[i2].ctype&2))
	{
		parts[i1].ctype |= 4;
		parts[i1].tmp2= i2;

		parts[i2].ctype |= 2;
		parts[i2].tmp = i1;
	}
}

constexpr float FREEZING = 248.15f;
constexpr float BLEND = 0.85f;

static int update(UPDATE_FUNC_ARGS)
{
	//0x01 - bubble on/off
	//0x02 - first mate yes/no
	//0x04 - "back" mate yes/no

	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (parts[i].ctype&1)
	{
		// reset invalid SOAP links
		if (!validIndex(parts[i].tmp) || !validIndex(parts[i].tmp2))
		{
			parts[i].tmp = parts[i].tmp2 = parts[i].ctype = 0;
			return 0;
		}
		if (parts[i].temp>FREEZING)
		{
			if (parts[i].life<=0)
			{
				//if only connected on one side
				if ((parts[i].ctype&6) != 6 && (parts[i].ctype&6))
				{
					int target = i;
					//break entire bubble in a loop
					while((parts[target].ctype&6) != 6 && (parts[target].ctype&6) && parts[target].type == PT_SOAP)
					{
						if (parts[target].ctype&2)
						{
							target = parts[target].tmp;
							if (!validIndex(target))
							{
								break;
							}
							Element_SOAP_detach(sim, target);
						}
						if (parts[target].ctype&4)
						{
							target = parts[target].tmp2;
							if (!validIndex(target))
							{
								break;
							}
							Element_SOAP_detach(sim, target);
						}
					}
				}
				if ((parts[i].ctype&6) != 6)
					parts[i].ctype = 0;
				if (validIndex(parts[i].tmp) && (parts[i].ctype&6) == 6 && (parts[parts[i].tmp].ctype&6) == 6 && parts[parts[i].tmp].tmp == i)
					Element_SOAP_detach(sim, i);
			}
			parts[i].vy = (parts[i].vy-0.1f)*0.5f;
			parts[i].vx *= 0.5f;
		}
		if(!(parts[i].ctype&2))
		{
			for (auto rx = -2; rx <= 2; rx++)
			{
				for (auto ry = -2; ry <= 2; ry++)
				{
					if (rx || ry)
					{
						auto r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if ((parts[ID(r)].type == PT_SOAP) && (parts[ID(r)].ctype&1) && !(parts[ID(r)].ctype&4))
							attach(parts, i, ID(r));
					}
				}
			}
		}
		else
		{
			if (parts[i].life<=0)
			{
				for (auto rx = -2; rx <= 2; rx++)
				{
					for (auto ry = -2; ry <= 2; ry++)
					{
						if (rx || ry)
						{
							auto r = pmap[y+ry][x+rx];
							if (!r && !sim->bmap[(y+ry)/CELL][(x+rx)/CELL])
								continue;
							if (parts[i].temp>FREEZING)
							{
								if (sim->bmap[(y+ry)/CELL][(x+rx)/CELL]
									|| (r && !(elements[TYP(r)].Properties&TYPE_GAS)
								    && TYP(r) != PT_SOAP && TYP(r) != PT_GLAS))
								{
									Element_SOAP_detach(sim, i);
									continue;
								}
							}
							if (TYP(r) == PT_SOAP)
							{
								if (parts[ID(r)].ctype == 1)
								{
									int buf = parts[i].tmp;

									parts[i].tmp = ID(r);
									if (validIndex(buf) && parts[buf].type == PT_SOAP)
										parts[buf].tmp2 = ID(r);
									parts[ID(r)].tmp2 = i;
									parts[ID(r)].tmp = buf;
									parts[ID(r)].ctype = 7;
								}
								else if (parts[ID(r)].ctype == 7 && parts[i].tmp != ID(r) && parts[i].tmp2 != ID(r))
								{
									if (validIndex(parts[i].tmp) && parts[parts[i].tmp].type == PT_SOAP)
										parts[parts[i].tmp].tmp2 = parts[ID(r)].tmp2;
									if (validIndex(parts[ID(r)].tmp2) && parts[parts[ID(r)].tmp2].type == PT_SOAP)
										parts[parts[ID(r)].tmp2].tmp = parts[i].tmp;
									parts[ID(r)].tmp2 = i;
									parts[i].tmp = ID(r);
								}
							}
						}
					}
				}
			}
		}
		if(parts[i].ctype&2 && validIndex(parts[i].tmp))
		{
			float d, dx, dy;
			dx = parts[i].x - parts[parts[i].tmp].x;
			dy = parts[i].y - parts[parts[i].tmp].y;
			d = 9/(pow(dx, 2)+pow(dy, 2)+9)-0.5;
			parts[parts[i].tmp].vx -= dx*d;
			parts[parts[i].tmp].vy -= dy*d;
			parts[i].vx += dx*d;
			parts[i].vy += dy*d;
			if ((parts[parts[i].tmp].ctype&2) && (parts[parts[i].tmp].ctype&1)
					&& validIndex(parts[parts[i].tmp].tmp)
					&& (parts[parts[parts[i].tmp].tmp].ctype&2) && (parts[parts[parts[i].tmp].tmp].ctype&1))
			{
				int ii = parts[parts[parts[i].tmp].tmp].tmp;
				if (validIndex(ii))
				{
					dx = parts[ii].x - parts[parts[i].tmp].x;
					dy = parts[ii].y - parts[parts[i].tmp].y;
					d = 81/(pow(dx, 2)+pow(dy, 2)+81)-0.5;
					parts[parts[i].tmp].vx -= dx*d*0.5f;
					parts[parts[i].tmp].vy -= dy*d*0.5f;
					parts[ii].vx += dx*d*0.5f;
					parts[ii].vy += dy*d*0.5f;
				}
			}
		}
	}
	else
	{
		if (sim->pv[y/CELL][x/CELL]>0.5f || sim->pv[y/CELL][x/CELL]<(-0.5f))
		{
			parts[i].ctype = 1;
			parts[i].life = 10;
		}

		//SOAP+OIL foam effect
		for (auto rx=-2; rx<3; rx++)
			for (auto ry=-2; ry<3; ry++)
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r) == PT_OIL)
					{
						float ax, ay, gx, gy;

						sim->GetGravityField(x, y, elements[PT_SOAP].Gravity, 1.0f, gx, gy);

						ax = ((parts[i].vx-gx)*0.5f + parts[ID(r)].vx)/2;
						ay = ((parts[i].vy-gy)*0.5f + parts[ID(r)].vy)/2;
						parts[i].vx = parts[ID(r)].vx = ax;
						parts[i].vy = parts[ID(r)].vy = ay;
					}
				}
	}
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r)!=PT_SOAP)
				{
					auto tr = float((parts[ID(r)].dcolour>>16)&0xFF);
					auto tg = float((parts[ID(r)].dcolour>>8)&0xFF);
					auto tb = float((parts[ID(r)].dcolour)&0xFF);
					auto ta = float((parts[ID(r)].dcolour>>24)&0xFF);
					auto nr = int(tr*BLEND);
					auto ng = int(tg*BLEND);
					auto nb = int(tb*BLEND);
					auto na = int(ta*BLEND);
					parts[ID(r)].dcolour = nr<<16 | ng<<8 | nb | na<<24;
				}
			}
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= EFFECT_LINES|PMODE_BLUR;
	return 1;
}

static void changeType(ELEMENT_CHANGETYPE_FUNC_ARGS)
{
	if (from == PT_SOAP && to != PT_SOAP)
	{
		Element_SOAP_detach(sim, i);
	}
}
