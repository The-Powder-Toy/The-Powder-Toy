#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_LITH PT_LITH 187
Element_LITH::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0X707070);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 10;
	Meltable = 1;
	Hardness = 10;

	Weight = 100;

	Temperature = R_TEMP + 273.15f;
	HeatConduct = 0;
	Description = "Lithium battery. Charge with INST when deactivated, discharges to INST when activated. (use Heat/Cool)";

	Properties = TYPE_SOLID;
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;


	Update = &Element_LITH::update;
	Graphics = &Element_LITH::graphics;
}

//#TPT-Directive ElementHeader Element_LITH static int update(UPDATE_FUNC_ARGS)
int Element_LITH::update(UPDATE_FUNC_ARGS)

{
	int r, rx, ry, charge, chargediffuse;
	//Prevent setting capacity below 1.
	if (parts[i].temp <= 1.0f + 273.15f)
		parts[i].temp = 1.0f + 273.15f;

	//Explosion code ( Gets ignored in powered mode)
	if (parts[i].tmp > parts[i].temp - 272.0f && parts[i].tmp2 != 1)
	{
		parts[i].type = PT_BOMB;
	}
	//Powered battery mode. (.tmp2 = 1 turns it into never discharging powered battery.)
	if (parts[i].tmp2 == 1)
	{
		parts[i].tmp = parts[i].temp - 273.15f;
	}
	//Activation and Deactivation.
	if (parts[i].life != 10)
	{
		if (parts[i].life > 0)
			parts[i].life--;
	}
	else
	{
		for (rx = -2; rx < 3; rx++)
			for (ry = -2; ry < 3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y + ry][x + rx];
					if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
						continue;
					if (TYP(r) == PT_LITH)
					{
						if (parts[ID(r)].life < 10 && parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[ID(r)].life == 0)
							parts[ID(r)].life = 10;
					}
				}
	}
	//Battery discharging.
	for (rx = -3; rx < 3; rx++)
		for (ry = -3; ry < 3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
					continue;
				switch (TYP(r))
				{
				case PT_INST:
					if (parts[i].tmp > 0 && parts[i].life == 0)
					{
						sim->create_part(ID(r), x + rx, y + ry, PT_SPRK);
						if (RNG::Ref().chance(1, 9))
						{
							parts[i].tmp -= 1;
						}
					}
					break;
					//Various reactions with different kinds of water elements. Slowly reacts with water and releases H2 gas.
				   //Exothermic reaction while reacting with water, heats nearby water as per its stored charge.
				case PT_WATR:
				case PT_SLTW:
				case PT_CBNW:
				case PT_DSTW:
					if (RNG::Ref().chance(1, 700))
					{
						sim->part_change_type(i, x, y, PT_H2);
						parts[i].temp += parts[ID(r)].tmp;
					}
					break;
				}
			}
	//Diffusion of tmp i.e stored charge.
	for (chargediffuse = 0; chargediffuse < 8; chargediffuse++)
	{
		rx = RNG::Ref().between(-2, 2);
		ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y + ry][x + rx];
			if (!r || sim->parts_avg(ID(r), i, PT_INSL) == PT_INSL)
				continue;
			if (TYP(r) == PT_LITH && (parts[i].tmp > parts[ID(r)].tmp) && parts[i].tmp > 0)//diffusion
			{
				charge = parts[i].tmp - parts[ID(r)].tmp;
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
//#TPT-Directive ElementHeader Element_LITH static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LITH::graphics(GRAPHICS_FUNC_ARGS)
{
	// Charging/discharging.
	if (cpart->life == 0)
	{
		*colr = 165;
		*colg = 165;
		*colb = 165;
	}
	if (cpart->tmp2 != 1)
	{
		if (cpart->tmp > 0)
		{
			int chargingstate = (int)(((float)cpart->tmp / (cpart->temp - 273.15))*100.0f);
			*colg += chargingstate;
			*colr -= chargingstate;
			*colb -= chargingstate;

		}
		if (cpart->tmp <= 0 && cpart->life == 0)
		{
			*colr = 255;
			*colg = 0;
			*colb = 0;
		}
	}
	//Powered battery mode.
	if (cpart->tmp2 == 1)
	{
		*colb = 255;
	}
	return 0;
}
Element_LITH::~Element_LITH() {}