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
	Description = "Lithium battery.(PSCN activates, NSCN deactivates, INST recieve/send charge, .tmp2=1 fast charges)";

	Properties = TYPE_SOLID | PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 3393.0f;
	HighTemperatureTransition = PT_BOMB;

	Update = &Element_LITH::update;
	Graphics = &Element_LITH::graphics;
}

//#TPT-Directive ElementHeader Element_LITH static int update(UPDATE_FUNC_ARGS)
int Element_LITH::update(UPDATE_FUNC_ARGS)

{                                                                                 //Basic code for activation and deactivation.
	int r, rx, ry,np;
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
	                                                                              //Code for LITH discharging.
	for (rx = -4; rx < 4; rx++)
		for (ry = -4; ry < 4; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y + ry][x + rx];
				switch (TYP(r))
				{
				case PT_INST:
					if (parts[i].temp > 274.15f && parts[i].life == 0)
					{
						parts[i].temp -= 0.15f;
						np = sim->create_part(ID(r), x + rx, y + ry, PT_SPRK);
					}
					break;
					//Various reactions with different kinds of water elements.Slowly reacts with water and releases H2 gas.
					case PT_WATR:
						if (RNG::Ref().chance(1, 700))
						{
							sim->part_change_type(i, x, y, PT_H2);
							parts[i].life = 65;
						}
					break;
					case PT_SLTW:
						if (RNG::Ref().chance(1, 700))
					{
						sim->part_change_type(i, x, y, PT_H2);
						parts[i].life = 65;
					}
					break;
					case PT_CBNW:
						if (RNG::Ref().chance(1, 700))
					{
						sim->part_change_type(i, x, y, PT_H2);
						parts[i].life = 65;
					}
					break;
					}
				}
	return 0;
}


//#TPT-Directive ElementHeader Element_LITH static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LITH::graphics(GRAPHICS_FUNC_ARGS)
{
	int gradv;
	double tempOver = (((cpart->temp)));
	
	if (cpart->temp >= 3293.0f )                            // Fancy colour states ( Green = Full, Red = Discharged, Dull Blue = Fast charge but OFF, Blue glow = Fast charge and on, White/Grey = On/OFF. )
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 0.0);
		*fireg = (int)(gradv * 250.0);
		*fireb = (int)(gradv * 0.0);
		*firea = 20;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	if (cpart->temp <= 275.0f)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 200.0);
		*fireg = (int)(gradv * 0.0);
		*fireb = (int)(gradv * 0.0);
		*firea = 20;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
		
	}
	if (cpart->life == 0 && cpart->temp <= 3293.0f && cpart->temp>= 280.0f && cpart-> tmp2 != 1 )
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 90.0);
		*fireg = (int)(gradv * 90.0);
		*fireb = (int)(gradv * 90.0);
		

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;

	}
	if (cpart->tmp2 == 1 && cpart->temp <= 3293.0f && cpart->life == 10)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 10.0);
		*fireg = (int)(gradv * 10.0);
		*fireb = (int)(gradv * 50.0);
		*firea = 5;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;
	}
	if (cpart->tmp2 == 1 && cpart->temp <= 3293.0f && cpart->life == 0)
	{
		double gradv = sin(tempOver) + 2.0;
		*firer = (int)(gradv * 0.0);
		*fireg = (int)(gradv * 0.0);
		*fireb = (int)(gradv * 200.0);
		*firea = 30;

		*colr += *firer;
		*colg += *fireg;
		*colb += *fireb;
		*pixel_mode |= FIRE_ADD;

	}
	return 0;
}
Element_LITH::~Element_LITH() {}
