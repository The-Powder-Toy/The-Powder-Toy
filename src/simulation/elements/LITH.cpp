#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0xB6AABF);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.96f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.2f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 15;

	Weight = 17;

	HeatConduct = 70;
	Description = "Lithium. Reactive element that explodes on contact with water.";

	Properties = TYPE_PART | PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 453.65f;
	HighTemperatureTransition = PT_LAVA;

	Update = &update;
	Graphics = &graphics;
}

/*

tmp2:  carbonation factor
life:  burn timer above 1000, spark cooldown timer otherwise
tmp:   hydrogenation factor
ctype: absorbed energy

For game reasons, baseline LITH has the reactions of both its pure form and
its hydroxide, and also has basic li-ion battery-like behavior.
It absorbs CO2 like its hydroxide form, but can only be converted into GLAS
after having absorbed CO2.

*/

static int update(UPDATE_FUNC_ARGS)
{
	Particle &self = parts[i];

	int &hydrogenationFactor = self.tmp;
	int &burnTimer = self.life;
	int &carbonationFactor = self.tmp2;
	int &storedEnergy = self.ctype;
	if (storedEnergy < 0)
	{
		storedEnergy = 0;
	}

	bool charged = false;
	bool discharged = false;
	for (int rx = -2; rx <= 2; ++rx)
	{
		for (int ry = -2; ry <= 2; ++ry)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				int neighborData = pmap[y + ry][x + rx];
				if (!neighborData)
				{
					if (burnTimer > 1012 && RNG::Ref().chance(1, 10))
					{
						sim->create_part(-1, x + rx, y + ry, PT_FIRE);
					}
					continue;
				}
				Particle &neighbor = parts[ID(neighborData)];

				switch (TYP(neighborData))
				{
				case PT_SLTW:
				case PT_WTRV:
				case PT_WATR:
				case PT_DSTW:
				case PT_CBNW:
					if (burnTimer > 1016)
					{
						sim->part_change_type(ID(neighborData), x + rx, y + ry, PT_WTRV);
						neighbor.temp = 440.f;
						continue;
					}
					if (hydrogenationFactor + carbonationFactor >= 10)
					{
						continue;
					}
					if (self.temp > 440.f)
					{
						burnTimer = 1024 + (storedEnergy > 24 ? 96 : storedEnergy * 4);
						sim->part_change_type(ID(neighborData), x + rx, y + ry, PT_H2);
						hydrogenationFactor = 10;
					}
					else
					{
						self.temp = restrict_flt(self.temp + 20.365f + storedEnergy * storedEnergy * 1.5f, MIN_TEMP, MAX_TEMP);
						sim->part_change_type(ID(neighborData), x + rx, y + ry, PT_H2);
						hydrogenationFactor += 1;
					}
					break;

				case PT_CO2:
					if (hydrogenationFactor + carbonationFactor >= 10)
					{
						continue;
					}
					sim->kill_part(ID(neighborData));
					carbonationFactor += 1;
					break;

				case PT_SPRK:
					if (sim->parts_avg(i, ID(neighborData), PT_INSL) == PT_INSL)
					{
						break;
					}
					if (hydrogenationFactor + carbonationFactor >= 5)
					{
						continue; // too impure to do battery things.
					}
					if (neighbor.ctype == PT_PSCN && neighbor.life == 3 && !charged && !burnTimer)
					{
						charged = true;
					}
					break;

				case PT_NSCN:
					if (sim->parts_avg(i, ID(neighborData), PT_INSL) == PT_INSL)
					{
						break;
					}
					if (neighbor.life == 0 && storedEnergy > 0 && !burnTimer)
					{
						sim->part_change_type(ID(neighborData), x + rx, y + ry, PT_SPRK);
						neighbor.life = 4;
						neighbor.ctype = PT_NSCN;
						discharged = true;
					}
					break;

				case PT_FIRE:
					if (self.temp > 440.f && RNG::Ref().chance(1, 40) && hydrogenationFactor < 6)
					{
						burnTimer = 1013;
						hydrogenationFactor += 1;
					}
					break;

				case PT_O2:
					if (burnTimer > 1000 && RNG::Ref().chance(1, 10))
					{
						sim->part_change_type(i, x, y, PT_PLSM);
						sim->part_change_type(ID(neighborData), x + rx, y + ry, PT_PLSM);
						sim->pv[y / CELL][x / CELL] += 4.0;
						return 0;
					}						
					break;
				}
			}
		}
	}
	if (charged)
	{
		storedEnergy += 1;
		burnTimer = 8;
	}
	if (discharged)
	{
		storedEnergy -= 1;
		burnTimer = 8;
	}

	for (int trade = 0; trade < 9; ++trade)
	{
		int rx = RNG::Ref().between(-3, 3);
		int ry = RNG::Ref().between(-3, 3);
		if (BOUNDS_CHECK && (rx || ry))
		{
			int neighborData = pmap[y + ry][x + rx];
			if (TYP(neighborData) != PT_LITH)
			{
				continue;
			}
			Particle &neighbor = parts[ID(neighborData)];

			int &neighborStoredEnergy = neighbor.ctype;
			// Transfer overcharge explosion status to nearby LITH
			if (burnTimer < 1000 && storedEnergy > 90 && neighbor.life > 1000)
				burnTimer = 1024;
			if (storedEnergy > neighborStoredEnergy)
			{
				int transfer = storedEnergy - neighborStoredEnergy;
				transfer -= transfer / 2;
				neighborStoredEnergy += transfer;
				storedEnergy -= transfer;
				break;
			}
		}
	}

	// Overcharged - begin explosion
	if (burnTimer < 1000 && storedEnergy >= 100)
		burnTimer = 1024;
	if (burnTimer == 1000)
	{
		burnTimer = 0;
		sim->part_change_type(i, x, y, PT_LAVA);
		if (carbonationFactor < 3)
		{
			self.temp = 500.f + storedEnergy * 10;
			self.ctype = PT_LITH;
		}
		else
		{
			self.temp = 2000.f + storedEnergy * 10;
			self.ctype = PT_GLAS;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Exploding lith
	if (cpart->life >= 1000)
	{
		int colour = 0xFFA040;
		*colr = PIXR(colour);
		*colg = PIXG(colour);
		*colb = PIXB(colour);
		*pixel_mode |= PMODE_FLARE | PMODE_GLOW;
	}
	// Charged lith
	else if (cpart->ctype > 0)
	{
		int mult = RNG::Ref().between(cpart->ctype / 3, cpart->ctype) / 15;
		mult = std::min(6, mult);
		*colr -= 30 * mult;
		*colb += 20 * mult;
		*pixel_mode |= PMODE_FLARE | PMODE_GLOW;
	}
	return 0;
}
