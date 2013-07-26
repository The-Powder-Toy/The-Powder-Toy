#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_LCRY PT_LCRY 54
Element_LCRY::Element_LCRY()
{
	Identifier = "DEFAULT_PT_LCRY";
	Name = "LCRY";
	Colour = PIXPACK(0x505050);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
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
	
	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Liquid Crystal. Changes colour when charged. (PSCN Charges, NSCN Discharges)";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 1273.0f;
	HighTemperatureTransition = PT_BGLA;
	
	Update = &Element_LCRY::update;
	Graphics = &Element_LCRY::graphics;
}

//#TPT-Directive ElementHeader Element_LCRY static int update(UPDATE_FUNC_ARGS)
int Element_LCRY::update(UPDATE_FUNC_ARGS)

{
	int r, rx, ry, check, setto;
	switch (parts[i].tmp)
	{
	case 1:
		if(parts[i].life<=0)
			parts[i].tmp = 0;
		else
		{
			parts[i].life-=2;
			if(parts[i].life < 0)
				parts[i].life = 0;
			parts[i].tmp2 = parts[i].life;
		}
	case 0:
		check=3;
		setto=1;
		break;
	case 2:
		if(parts[i].life>=10)
			parts[i].tmp = 3;
		else
		{
			parts[i].life+=2;
			if(parts[i].life > 10)
				parts[i].life = 10;
			parts[i].tmp2 = parts[i].life;
		}
	case 3:
		check=0;
		setto=2;
		break;
	default:
		return 0;
	}
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_LCRY && parts[r>>8].tmp == check)
				{
					parts[r>>8].tmp = setto;
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_LCRY static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LCRY::graphics(GRAPHICS_FUNC_ARGS)

{
	if(ren->decorations_enable && cpart->dcolour && (cpart->dcolour&0xFF000000))
	{
		*colr = (cpart->dcolour>>16)&0xFF;
		*colg = (cpart->dcolour>>8)&0xFF;
		*colb = (cpart->dcolour)&0xFF;

		if(cpart->tmp2<10){
			*colr /= 10-cpart->tmp2;
			*colg /= 10-cpart->tmp2;
			*colb /= 10-cpart->tmp2;
		}
		
	}
	else
	{
		*colr = *colg = *colb = 0x50+((cpart->tmp2>10?10:cpart->tmp2)*10);
	}
	*pixel_mode |= NO_DECO;
	return 0;
					
	/*int lifemod = ((cpart->tmp2>10?10:cpart->tmp2)*10);
	*colr += lifemod; 
	*colg += lifemod; 
	*colb += lifemod; 
	if(decorations_enable && cpart->dcolour && cpart->dcolour&0xFF000000)
	{
		lifemod *= 2.5f;
		if(lifemod < 40)
			lifemod = 40;
		*colr = (lifemod*((cpart->dcolour>>16)&0xFF) + (255-lifemod)**colr) >> 8;
		*colg = (lifemod*((cpart->dcolour>>8)&0xFF) + (255-lifemod)**colg) >> 8;
		*colb = (lifemod*((cpart->dcolour)&0xFF) + (255-lifemod)**colb) >> 8;
	}
	*pixel_mode |= NO_DECO;
	return 0;*/
}


Element_LCRY::~Element_LCRY() {}
