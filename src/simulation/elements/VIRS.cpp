#include "simulation/ElementCommon.h"
#include "VIRS.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_VIRS()
{
	Identifier = "DEFAULT_PT_VIRS";
	Name = "VIRS";
	Colour = 0xFE11F6_rgb;
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

	DefaultProperties.temp = 72.0f + 273.15f;
	HeatConduct = 251;
	Description = "Virus. Turns everything it touches into virus.";

	Properties = TYPE_LIQUID|PROP_DEADLY;
	CarriesTypeIn = 1U << FIELD_TMP2;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 305.0f;
	LowTemperatureTransition = PT_VRSS;
	HighTemperature = 673.0f;
	HighTemperatureTransition = PT_VRSG;

	DefaultProperties.tmp4 = 250;

	Update = &Element_VIRS_update;
	Graphics = &graphics;
}

int Element_VIRS_update(UPDATE_FUNC_ARGS)
{
	//tmp3 measures how many frames until it is cured (0 if still actively spreading and not being cured)
	//tmp4 measures how many frames until it dies
	int rndstore = sim->rng.gen();
	if (parts[i].tmp3)
	{
		parts[i].tmp3 -= (rndstore & 0x1) ? 0:1;
		//has been cured, so change back into the original element
		if (!parts[i].tmp3)
		{
			sim->part_change_type(i,x,y,parts[i].tmp2);
			parts[i].tmp2 = 0;
			parts[i].tmp3 = 0;
			parts[i].tmp4 = 0;
		}
		return 0;
		//cured virus is never in below code
	}
	//decrease tmp4 so it slowly dies
	if (parts[i].tmp4)
	{
		if (!(rndstore & 0x7) && --parts[i].tmp4 <= 0)
		{
			sim->kill_part(i);
			return 1;
		}
		rndstore >>= 3;
	}

	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			if (rx || ry)
			{
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;

				//spread "being cured" state
				if (parts[ID(r)].tmp3 && (TYP(r) == PT_VIRS || TYP(r) == PT_VRSS || TYP(r) == PT_VRSG))
				{
					parts[i].tmp3 = parts[ID(r)].tmp3 + ((rndstore & 0x3) ? 2:1);
					return 0;
				}
				//soap cures virus
				else if (TYP(r) == PT_SOAP)
				{
					parts[i].tmp3 += 10;
					if (!(rndstore & 0x3))
						sim->kill_part(ID(r));
					return 0;
				}
				else if (TYP(r) == PT_PLSM)
				{
					if (surround_space && sim->rng.chance(10 + int(sim->pv[(y+ry)/CELL][(x+rx)/CELL]), 100))
					{
						sim->create_part(i, x, y, PT_PLSM);
						return 1;
					}
				}
				//transforms things into virus here
				else if (TYP(r) != PT_VIRS && TYP(r) != PT_VRSS && TYP(r) != PT_VRSG && TYP(r) != PT_DMND && TYP(r) != PT_BASE)
				{
					if (!(rndstore & 0x7))
					{
						parts[ID(r)].tmp2 = TYP(r);
						parts[ID(r)].tmp3 = 0;
						if (parts[i].tmp4)
							parts[ID(r)].tmp4 = parts[i].tmp4 + 1;
						else
							parts[ID(r)].tmp4 = 0;
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
					parts[i].tmp4 = 0;
				}
			}
			//reset rndstore only once, halfway through
			else if (!rx && !ry)
				rndstore = sim->rng.gen();
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*pixel_mode |= PMODE_BLUR;
	*pixel_mode |= NO_DECO;
	return 1;
}
