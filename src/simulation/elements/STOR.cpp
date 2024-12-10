#include "simulation/ElementCommon.h"
#include "SOAP.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static bool ctypeDraw(CTYPEDRAW_FUNC_ARGS);

void Element::Element_STOR()
{
	Identifier = "DEFAULT_PT_STOR";
	Name = "STOR";
	Colour = 0x50DFDF_rgb;
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

	HeatConduct = 0;
	Description = "Storage. Captures and stores a single particle. Releases when charged with PSCN, also passes to PIPE.";

	Properties = TYPE_SOLID | PROP_NOCTYPEDRAW;
	CarriesTypeIn = (1U << FIELD_CTYPE) | (1U << FIELD_TMP);

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
	CtypeDraw = &ctypeDraw;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (!sd.IsElementOrNone(parts[i].tmp))
		parts[i].tmp = 0;
	if(parts[i].life && !parts[i].tmp)
		parts[i].life--;
	for (auto rx = -2; rx <= 2; rx++)
	{
		for (auto ry = -2; ry <= 2; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					r= sim->photons[y+ry][x+rx];
				if (!r)
					continue;
				if (!parts[i].tmp && !parts[i].life && TYP(r)!=PT_STOR && !(elements[TYP(r)].Properties&TYPE_SOLID) && (!parts[i].ctype || TYP(r)==parts[i].ctype))
				{
					if (TYP(r) == PT_SOAP)
						Element_SOAP_detach(sim, ID(r));
					parts[i].tmp = parts[ID(r)].type;
					parts[i].temp = parts[ID(r)].temp;
					parts[i].tmp2 = parts[ID(r)].life;
					parts[i].tmp3 = parts[ID(r)].tmp;
					parts[i].tmp4 = parts[ID(r)].ctype;
					sim->kill_part(ID(r));
				}
				if(parts[i].tmp && TYP(r)==PT_SPRK && parts[ID(r)].ctype==PT_PSCN && parts[ID(r)].life>0 && parts[ID(r)].life<4)
				{
					for(auto ry1 = 1; ry1 >= -1; ry1--)
					{
						for(auto rx1 = 0; rx1 >= -1 && rx1 <= 1; rx1 = -rx1-rx1+1) // Oscillate the X starting at 0, 1, -1, 3, -5, etc (Though stop at -1)
						{
							auto np = sim->create_part(-1,x+rx1,y+ry1,TYP(parts[i].tmp));
							if (np!=-1)
							{
								parts[np].temp = parts[i].temp;
								parts[np].life = parts[i].tmp2;
								parts[np].tmp = parts[i].tmp3;
								parts[np].ctype = parts[i].tmp4;
								parts[i].tmp = 0;
								parts[i].life = 10;
								break;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
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

static bool ctypeDraw(CTYPEDRAW_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (elements[t].Properties & TYPE_SOLID)
	{
		return false;
	}
	return Element::basicCtypeDraw(CTYPEDRAW_FUNC_SUBCALL_ARGS);
}
