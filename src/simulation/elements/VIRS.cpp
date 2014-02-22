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
	
	State = ST_LIQUID;
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
		parts[i].pavg[0] -= (rndstore&0x1) ? 0:1;
		//has been cured, so change back into the original element
		if (!parts[i].pavg[0])
		{
			sim->part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].pavg[0] = 0;
			parts[i].pavg[1] = 0;
			return 0;
		}
	}
	//decrease pavg[1] so it slowly dies
	if (parts[i].pavg[1] > 0)
	{
		if (((rndstore>>1)&0xD) < 1)
		{
			parts[i].pavg[1]--;
			//if pavg[1] is now 0 and it's not in the process of being cured, kill it
			if (!parts[i].pavg[1] && !parts[i].pavg[0])
			{
				sim->kill_part(i);
				return 1;
			}
		}
	}

	//none of the things in the below loop happen while virus is being cured
	if (parts[i].pavg[0])
		return 0;

	for (rx=-1; rx<2; rx++)
	{
		//reset rndstore, one random can last through 3 locations and reduce rand() calling by up to 6x as much
		rndstore = rand();
		for (ry=-1; ry<2; ry++)
		{
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				//spread "being cured" state
				if (((r&0xFF) == PT_VIRS || (r&0xFF) == PT_VRSS || (r&0xFF) == PT_VRSG) && parts[r>>8].pavg[0])
				{
					parts[i].pavg[0] = parts[r>>8].pavg[0] + (((rndstore&0x7)>>1) ? 2:1);
					rndstore = rndstore >> 3;
					return 0;
				}
				//soap cures virus
				else if ((r&0xFF) == PT_SOAP)
				{
					parts[i].pavg[0] += 10;
					if (!((rndstore&0x7)>>1))
						sim->kill_part(r>>8);
					return 0;
				}
				else if ((r&0xFF) == PT_PLSM)
				{
					if (surround_space && 10 + (int)(sim->pv[(y+ry)/CELL][(x+rx)/CELL]) > (rand()%100))
					{
						sim->create_part(i, x, y, PT_PLSM);
						return 1;
					}
				}
				//transforms things into virus here
				else if ((r&0xFF) != PT_VIRS && (r&0xFF) != PT_VRSS && (r&0xFF) != PT_VRSG && (r&0xFF) != PT_DMND)
				{
					if (!((rndstore&0xF)>>1))
					{
						parts[r>>8].tmp2 = (r&0xFF);
						parts[r>>8].pavg[0] = 0;
						if (parts[i].pavg[1])
							parts[r>>8].pavg[1] = parts[i].pavg[1] + ((rndstore>>4) ? 1:0);
						else
							parts[r>>8].pavg[1] = 0;
						if (parts[r>>8].temp < 305.0f)
							sim->part_change_type(r>>8, x+rx, y+ry, PT_VRSS);
						else if (parts[r>>8].temp > 673.0f)
							sim->part_change_type(r>>8, x+rx, y+ry, PT_VRSG);
						else
							sim->part_change_type(r>>8, x+rx, y+ry, PT_VIRS);
					}
					rndstore = rndstore >> 5;
				}
				//protons make VIRS last forever
				else if ((sim->photons[y+ry][x+rx]&0xFF) == PT_PROT)
				{
					parts[i].pavg[1] = 0;
				}
			}
		}
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
