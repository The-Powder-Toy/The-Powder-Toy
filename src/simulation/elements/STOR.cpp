#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_STOR PT_STOR 83
Element_STOR::Element_STOR()
{
	Identifier = "DEFAULT_PT_STOR";
	Name = "STOR";
	Colour = PIXPACK(0x50DFDF);
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
	HeatConduct = 0;
	Description = "Storage. Captures and stores a single particle. Releases when charged with PSCN, also passes to PIPE.";

	Properties = TYPE_SOLID|PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_STOR::update;
	Graphics = &Element_STOR::graphics;
}

//#TPT-Directive ElementHeader Element_STOR static int update(UPDATE_FUNC_ARGS)
int Element_STOR::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np, rx1, ry1;
	if (!sim->IsValidElement(parts[i].tmp))
		parts[i].tmp = 0;
	if(parts[i].life && !parts[i].tmp)
		parts[i].life--;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((ID(r))>=NPART || !r)
					continue;
				if (!parts[i].tmp && !parts[i].life && TYP(r)!=PT_STOR && !(sim->elements[TYP(r)].Properties&TYPE_SOLID) && (!parts[i].ctype || TYP(r)==parts[i].ctype))
				{
					if (TYP(r) == PT_SOAP)
						Element_SOAP::detach(sim, ID(r));
					parts[i].tmp = parts[ID(r)].type;
					parts[i].temp = parts[ID(r)].temp;
					parts[i].tmp2 = parts[ID(r)].life;
					parts[i].pavg[0] = parts[ID(r)].tmp;
					parts[i].pavg[1] = parts[ID(r)].ctype;
					sim->kill_part(ID(r));
				}
				if(parts[i].tmp && TYP(r)==PT_SPRK && parts[ID(r)].ctype==PT_PSCN && parts[ID(r)].life>0 && parts[ID(r)].life<4)
				{
					for(ry1 = 1; ry1 >= -1; ry1--){
						for(rx1 = 0; rx1 >= -1 && rx1 <= 1; rx1 = -rx1-rx1+1){ // Oscillate the X starting at 0, 1, -1, 3, -5, etc (Though stop at -1)
							np = sim->create_part(-1,x+rx1,y+ry1,TYP(parts[i].tmp));
							if (np!=-1)
							{
								parts[np].temp = parts[i].temp;
								parts[np].life = parts[i].tmp2;
								parts[np].tmp = parts[i].pavg[0];
								parts[np].ctype = parts[i].pavg[1];
								parts[i].tmp = 0;
								parts[i].life = 10;
								break;
							}
						}
					}
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_STOR static int graphics(GRAPHICS_FUNC_ARGS)
int Element_STOR::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->tmp){
		*pixel_mode |= PMODE_GLOW;
		*colr = 0x50;
		*colg = 0xDF;
		*colb = 0xDF;
	} else {
		*colr = 0x20;
		*colg = 0xAF;
		*colb = 0xAF;
	}
	return 0;
}


Element_STOR::~Element_STOR() {}
