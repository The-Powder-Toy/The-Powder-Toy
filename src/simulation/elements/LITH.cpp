#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0xC0C0C0);
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
	Hardness = 30;

	Weight = 17;

	HeatConduct = 70;
	Description = "Lithium. Reactive element that explodes on contact with water.";

	Properties = TYPE_PART|PROP_LIFE_DEC;
	
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

/* Element property allocations
*  tmp2: carbonation factor
*  life:  burn timer
*  tmp:   hydrogenation factor
*  ctype:  absorbed energy
*/

/* General docs
* For game reasons, baseline LITH has the reactions of both it's pure form and
* it's hydroxide, and also has basic li-ion battery-like behavior.
* It absorbs CO2 like it's hydroxide form, but can only be converted into GLAS
* after having absorbed CO2.
*
* If LITH comes in contact with water, it will consume 1 WATR, increment tmp,
* and heat itself up by 400K. At 1000K it bursts into flames and sets tmp to 10
* if still in contact with WATR, setting it's life to 24 and insta-boiling
* water in it's immediate vincity. 
*/

static int update(UPDATE_FUNC_ARGS)
{
	Particle& self = parts[i];

	int& hydrogenation_factor = self.tmp;
	int& burn_timer = self.life;
	int& carbonation_factor = self.tmp2;
	int& stored_energy = self.ctype;
	if (stored_energy < 0)
		stored_energy = 0;
		

	for (int rx = -1; rx < 2; rx++)
		for (int ry = -1; ry < 2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int neighbour_data = pmap[y+ry][x+rx];

				if (!neighbour_data) {
					if (self.life > 12 && RNG::Ref().chance(1,10))
						sim->create_part(-1, x + rx, y + ry, PT_FIRE);
					continue;
				}
				
				Particle& neighbour = parts[ID(neighbour_data)];

				switch (TYP(neighbour_data)) {
					case PT_SLTW:
					case PT_WTRV:
					case PT_WATR:
					case PT_DSTW:
					case PT_CBNW:
						if (self.life > 16) {
							sim->part_change_type(ID(neighbour_data), x + rx, y + ry, PT_WTRV);
							
							neighbour.temp = 453.65f;
							continue;
						} 

						if (hydrogenation_factor + carbonation_factor >= 10)
							continue;
						if (self.temp > 453.65) {
							self.life = 24 + (stored_energy > 24 ? 24 : stored_energy);
							sim->part_change_type(ID(neighbour_data), x + rx, y + ry, PT_H2);
							hydrogenation_factor = 10;
						} else {
							self.temp += restrict_flt(20.365f + stored_energy * (stored_energy * 1.5f), MIN_TEMP, MAX_TEMP);
							sim->part_change_type(ID(neighbour_data), x + rx, y + ry, PT_H2);
							hydrogenation_factor += 1;
						}
						break;
					case PT_CO2:
						if (hydrogenation_factor + carbonation_factor >= 10)
							continue;
						sim->kill_part(ID(neighbour_data));
						carbonation_factor += 1;
						break;
					case PT_SPRK:
						if (hydrogenation_factor + carbonation_factor >= 5)
							continue; // too impure to do battery things.
						if (neighbour.ctype == PT_PSCN && neighbour.life == 4 && RNG::Ref().chance(1,10)) {
							stored_energy += 1;
						}
						break;
					case PT_NSCN:
						if (neighbour.life == 0 && stored_energy > 2)
						{
							sim->part_change_type(ID(neighbour_data), x + rx, y + ry, PT_SPRK);
							neighbour.life = 4;
							neighbour.ctype = PT_NSCN;
							stored_energy -= 2;
						}
						break;
					case PT_FIRE:
						if (self.temp > 543.0f && RNG::Ref().chance(1,40) && hydrogenation_factor < 6)
						{
							self.life = 13;
							hydrogenation_factor += 1;
						}
				}
			}

	for (int trade = 0; trade < 9; trade++)
	{
		int rx = RNG::Ref().between(-3, 3);
		int ry = RNG::Ref().between(-3, 3);
		if (BOUNDS_CHECK && (rx || ry))
		{
			int nghbr_dat = pmap[y + ry][x + rx];
			if (TYP(nghbr_dat) != PT_LITH)
				continue;
			Particle& ngbhr = parts[ID(nghbr_dat)];

			int& neighbour_stored_energy = ngbhr.ctype;

			if (stored_energy > neighbour_stored_energy)
			{
				int transfer = stored_energy - neighbour_stored_energy;
				neighbour_stored_energy += transfer / 2;
				stored_energy -= transfer / 2;
				break;
			}
		}
	}
	if (self.temp > 453.65f && self.life == 0) 
	{
		
		sim->part_change_type(i, x, y, PT_LAVA);
		if (carbonation_factor < 3) {
			self.ctype = PT_LITH;
		} else {
			self.ctype = PT_GLAS;
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->life != 0) {
		*colr = PIXR(0xFFA040);
		*colg = PIXG(0xFFA040);
		*colb = PIXB(0xFFA040);
		*pixel_mode |= PMODE_FLARE | PMODE_GLOW;
	}

	return 0;
}

