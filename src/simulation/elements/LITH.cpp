#include "simulation/ElementCommon.h"
//#TPT-Directive ElementClass Element_LITH PT_LITH 192
Element_LITH::Element_LITH()
{
	Identifier = "DEFAULT_PT_LITH";
	Name = "LITH";
	Colour = PIXPACK(0X707070);
	MenuVisible = 1;
	MenuSection = SC_CRACKER1000;
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

	Temperature = R_TEMP + 0.0f + 273.15f;
	HeatConduct = 254;
	Description = "Lithium battery.(PSCN activates, NSCN deactivates, INST recieve/send charge, .tmp2 sets capacity)";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3393.0f;
	HighTemperatureTransition = PT_BOMB; // Explodes when overheated.

	Update = &Element_LITH::update;
	Graphics = &Element_LITH::graphics;
}

//#TPT-Directive ElementHeader Element_LITH static int update(UPDATE_FUNC_ARGS)
int Element_LITH::update(UPDATE_FUNC_ARGS)

{
	int r, rx, ry, charge, chargediffuse, capacity = parts[i].tmp2;
	if (parts[i].tmp2 < 1)   //Prevent setting capacity below 1.
	{
		parts[i].tmp2 = 1;
	}
	if (parts[i].tmp > parts[i].tmp2)   //Explodes when overcharged.
	{
		parts[i].temp = 3395.0f;
	}

	//Basic code for activation and deactivation.
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
					if (!r)
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
	//Code for LITH battery discharging.

	for (rx = -4; rx < 4; rx++)
		for (ry = -4; ry < 4; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
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
					//Various reactions with different kinds of water elements.Slowly reacts with water and releases H2 gas.
					//Exothermic reaction while reacting with water.
				case PT_WATR:
				case PT_SLTW:
				case PT_CBNW:
					if (RNG::Ref().chance(1, 700))
					{
						sim->part_change_type(i, x, y, PT_H2);
						parts[i].life = 65;
						parts[i].temp += 10;
					}
					break;
				}
			}
	//Diffusion of tmp i.e stored charge.

	for (chargediffuse = 0; chargediffuse < 9; chargediffuse++)
	{
		rx = RNG::Ref().between(-2, 2);
		ry = RNG::Ref().between(-2, 2);
		if (BOUNDS_CHECK && (rx || ry))
		{
			r = pmap[y + ry][x + rx];
			if (!r)
				continue;
			if (TYP(r) == PT_LITH && (parts[i].tmp > parts[ID(r)].tmp) && parts[i].tmp > 0)//diffusion
			{
				charge = parts[i].tmp - parts[ID(r)].tmp;
				if (charge == 1)
				{
					parts[ID(r)].tmp++;
					parts[i].tmp--;
					chargediffuse = 9;
				}
				else if (charge > 0)
				{
					parts[ID(r)].tmp += charge / 2;
					parts[i].tmp -= charge / 2;
					chargediffuse = 9;
				}
			}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_LITH static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LITH::graphics(GRAPHICS_FUNC_ARGS)
{
	int gradv;
	double tempOver = (((cpart->tmp)));

	//Almost charged.
	if (cpart->tmp >= cpart->tmp2 - 4)                            // Fancy colour changes.
	{
		double gradv = sin(tempOver) + 2.0;
		*fireg = (int)(gradv * 250.0);
		*firea = 12;
		*colg += *fireg;
	}
	//Discharged.
	if (cpart->tmp <= 0 && cpart->life == 0)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 250.0);
		*firea = 12;
		*colr += *firer;
	}
	//Activated/Discharging.
	if (cpart->life == 0 && cpart->tmp != 0 && cpart->tmp != cpart->tmp2)
	{
		double gradv = sin(tempOver) + 2.0;
		*colr = 255.0;
		*colg = 255.0;
		*colb = 255.0;
	}
	*pixel_mode |= FIRE_ADD;

	return 0;
}
Element_LITH::~Element_LITH() {}
