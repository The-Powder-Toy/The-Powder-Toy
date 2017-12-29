#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VIRS PT_VIRS 174
Element_VIRS::Element_VIRS()
{
	Identifier = "DEFAULT_PT_VIRS";
	Name = "VIRS";
	Colour = PIXPACK(0xFE11F6);
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
	Hardness = 20;

	Weight = 31;

	Temperature = 72.0f	+ 273.15f;
	HeatConduct = 251;
	Description = "Virus. Turns everything it touches into virus.";

	Properties = TYPE_LIQUID|PROP_DEADLY;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 305.0f;
	LowTemperatureTransition = PT_VRSS;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_VRSG;

	Update = &Element_VIRS::update;
	Graphics = &Element_VIRS::graphics;
}

//#TPT-Directive ElementHeader Element_VIRS static int update(UPDATE_FUNC_ARGS)
int Element_VIRS::update(UPDATE_FUNC_ARGS)
{
	//pavg[0] measures how many frames until it is cured (0 if still actively spreading and not being cured)
	//pavg[1] measures how many frames until it dies
	int r, rx, ry, rndstore = rand();
	if (parts[i].pavg[0])
	{
		parts[i].pavg[0] -= (rndstore & 0x1) ? 0:1;
		//has been cured, so change back into the original element
		if (!parts[i].pavg[0])
		{
			sim->part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].pavg[0] = 0;
			parts[i].pavg[1] = 0;
		}
		return 0;
		//cured virus is never in below code
	}
	//decrease pavg[1] so it slowly dies
	if (parts[i].pavg[1])
	{
		if (!(rndstore & 0x7) && --parts[i].pavg[1] <= 0)
		{
			sim->kill_part(i);
			return 1;
		}
		rndstore >>= 3;
	}

	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				//spread "being cured" state
				if (parts[ID(r)].pavg[0] && (TYP(r) == PT_VIRS || TYP(r) == PT_VRSS || TYP(r) == PT_VRSG))
				{
					parts[i].pavg[0] = parts[ID(r)].pavg[0] + ((rndstore & 0x3) ? 2:1);
					return 0;
				}
				//soap cures virus
				else if (TYP(r) == PT_SOAP)
				{
					parts[i].pavg[0] += 10;
					if (!(rndstore & 0x3))
						sim->kill_part(ID(r));
					return 0;
				}
				else if (TYP(r) == PT_PLSM)
				{
					if (surround_space && 10 + (int)(sim->pv[(y+ry)/CELL][(x+rx)/CELL]) > (rand()%100))
					{
						sim->create_part(i, x, y, PT_PLSM);
						return 1;
					}
				}
				//transforms things into virus here
				else if (TYP(r) != PT_VIRS && TYP(r) != PT_VRSS && TYP(r) != PT_VRSG && TYP(r) != PT_DMND)
				{
					if (!(rndstore & 0x7))
					{
						parts[ID(r)].tmp2 = TYP(r);
						parts[ID(r)].pavg[0] = 0;
						if (parts[i].pavg[1])
							parts[ID(r)].pavg[1] = parts[i].pavg[1] + 1;
						else
							parts[ID(r)].pavg[1] = 0;
						if (parts[ID(r)].temp < 305.0f)
							sim->part_change_type(ID(r), x+rx, y+ry, PT_VRSS);
						else if (parts[ID(r)].temp > 673.0f)
							sim->part_change_type(ID(r), x+rx, y+ry, PT_VRSG);
						else
							sim->part_change_type(ID(r), x+rx, y+ry, PT_VIRS);
					}
					rndstore >>= 3;
				}
				//protons make VIRS last forever
				else if (TYP(sim->photons[y+ry][x+rx]) == PT_PROT)
				{
					parts[i].pavg[1] = 0;
				}
			}
			//reset rndstore only once, halfway through
			else if (!rx && !ry)
				rndstore = rand();
		}
	return 0;
}

//#TPT-Directive ElementHeader Element_VIRS static int graphics(GRAPHICS_FUNC_ARGS)
int Element_VIRS::graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_BLUR;
	*pixel_mode |= NO_DECO;
	return 1;
}

Element_VIRS::~Element_VIRS() {}
