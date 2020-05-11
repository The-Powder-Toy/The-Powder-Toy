#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0xFF0000);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
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
	Explosive = 1;
	Meltable = 0;
	Hardness = 15;

	Weight = 100;

	HeatConduct = 200;
	Description = "Lithium battery. Charge with INST when deactivated, discharges to INST when activated. (use Heat/Cool)";

	Properties = TYPE_SOLID;
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 454.15f;
	HighTemperatureTransition = PT_FIRE;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)

{
	//Prevent setting capacity below 1.
	if (parts[i].life <= 1)
		parts[i].life = 1;

	//Explosion code ( Gets ignored in powered mode)
	if (parts[i].tmp > parts[i].life)
	{
		parts[i].type = PT_FIRE;
	}
	//Activation and Deactivation.
	if (parts[i].tmp2 != 10)
	{
		if (parts[i].tmp2 > 0)
			parts[i].tmp2--;
	}

	else
	{
		for (int rx = -2; rx < 3; rx++)
			for (int ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y + ry][x + rx];
					if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
						continue;
					if (TYP(r) == PT_LITH)
					{
						if (parts[ID(r)].tmp2 < 10 && parts[ID(r)].tmp2>0)
							parts[i].tmp2 = 9;
						else if (parts[ID(r)].tmp2 == 0)
							parts[ID(r)].tmp2 = 10;
					}
				}
	}
	for (int rx = -2; rx < 3; rx++)
		for (int ry = -2; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				//Battery discharging.
				switch (TYP(r))
				{
					case PT_INST:
						if (parts[i].tmp > 0 && parts[i].tmp2 == 0)
						{
							parts[i].tmp -= 1;
							sim->FloodINST(x + rx, y + ry);
						}
						break;
					//Various reactions with different kinds of water elements. Slowly reacts with water and releases H2 gas.
				   //Exothermic reaction while reacting with water, heats nearby water as per its stored charge.
				case PT_WATR:
				case PT_SLTW:
				case PT_CBNW:
				case PT_DSTW:
				{  if (RNG::Ref().chance(1, 30))
				{
					parts[i].type = PT_H2;
					parts[ID(r)].type = PT_BRMT;
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] += 1.0;
					if (parts[ID(r)].tmp >= 22)
						parts[i].temp += parts[ID(r)].tmp;
				}
				}
				break;
				case PT_O2:
				{
					parts[i].type = PT_PLSM;
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] += 4.0;
				}
				break;
				case PT_ACID:
				{

					parts[i].type = PT_H2;
					sim->pv[(y / CELL) + ry][(x / CELL) + rx] += 0.5;
					if (RNG::Ref().chance(1, 800))
					{

						parts[i].type = PT_FIRE;
					}
				}
				break;
				}
			}
	//Diffusion of tmp i.e stored charge.
	for (int chargediffuse = 0; chargediffuse < 8; chargediffuse++)
	{
		int rx = RNG::Ref().between(-2, 2);
		int ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			int r = pmap[y + ry][x + rx];
			if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
				continue;
			if (TYP(r) == PT_LITH && (parts[i].tmp > parts[ID(r)].tmp) && parts[i].tmp > 0)//diffusion
			{
				int charge = parts[i].tmp - parts[ID(r)].tmp;
				if (charge == 1)
				{
					parts[ID(r)].tmp++;
					parts[i].tmp--;
					chargediffuse = 8;
				}
				else if (charge > 0)
				{
					parts[ID(r)].tmp += charge / 2;
					parts[i].tmp -= charge / 2;
					chargediffuse = 8;
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Charging/discharging.
	{	int chargingstate = (int)(((float)cpart->tmp / (cpart->life))*100.0f);
	*colg += chargingstate * 3;
	*colr -= chargingstate * 2;
	*colb -= chargingstate * 2;
	}

	if (cpart->tmp2 == 0)
	{
		*colr += 40;
		*colg += 40;
		*colb += 40;
	}

		
	if (*colg > 255)
	{
		*colg = 255;
	}
	return 0;
}
static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	sim->parts[i].life = 100;
}
