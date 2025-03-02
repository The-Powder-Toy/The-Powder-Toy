#include "simulation/ElementCommon.h"
#include "FIRE.h"
#include "FILT.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_PHOT()
{
	Identifier = "DEFAULT_PT_PHOT";
	Name = "PHOT";
	Colour = 0xFFFFFF_rgb;
	MenuVisible = 1;
	MenuSection = SC_NUCLEAR;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 1.00f;
	Loss = 1.00f;
	Collision = -0.99f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = -1;

	DefaultProperties.temp = R_TEMP + 900.0f + 273.15f;
	HeatConduct = 251;
	Description = "Photons. Refract through glass, and different elements change its color. Can ignite flammable materials.";

	Properties = TYPE_ENERGY | PROP_PHOTPASS | PROP_LIFE_DEC | PROP_LIFE_KILL_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 680;
	DefaultProperties.ctype = 0x3FFFFFFF;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;

	if (!(parts[i].ctype&0x3FFFFFFF)) {
		sim->kill_part(i);
		return 1;
	}
	if (parts[i].temp > 506)
		if (sim->rng.chance(1, 10))
			Element_FIRE_update(UPDATE_FUNC_SUBCALL_ARGS);
	for (auto rx = -1; rx <= 1; rx++)
	{
		for (auto ry = -1; ry <= 1; ry++)
		{
			auto r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			if (TYP(r)==PT_ISOZ || TYP(r)==PT_ISZS)
			{
				if (sim->rng.chance(1, 400))
				{
					parts[i].vx *= 0.90f;
					parts[i].vy *= 0.90f;
					sim->create_part(ID(r), x+rx, y+ry, PT_PHOT);
					auto rrr = sim->rng.between(0, 359) * 3.14159f / 180.0f;
					int rr;
					if (TYP(r) == PT_ISOZ)
						rr = int(sim->rng.between(128, 255) / 127.0f);
					else
						rr = int(sim->rng.between(128, 355) / 127.0f);
					parts[ID(r)].vx = rr*cosf(rrr);
					parts[ID(r)].vy = rr*sinf(rrr);
					sim->pv[y/CELL][x/CELL] -= 15.0f * CFDS;
				}
			}
			else if((TYP(r) == PT_QRTZ || TYP(r) == PT_PQRT) && !ry && !rx)//if on QRTZ
			{
				float a = sim->rng.between(0, 359) * 3.14159f / 180.0f;
				parts[i].vx = 3.0f*cosf(a);
				parts[i].vy = 3.0f*sinf(a);
				if(parts[i].ctype == 0x3FFFFFFF)
					parts[i].ctype = 0x1F << sim->rng.between(0, 25);
				if (parts[i].life)
					parts[i].life++; //Delay death
			}
			else if(TYP(r) == PT_BGLA && !ry && !rx)//if on BGLA
			{
				float a = sim->rng.between(-50, 50) * 0.001f;
				float rx = cosf(a), ry = sinf(a), vx, vy;
				vx = rx * parts[i].vx + ry * parts[i].vy;
				vy = rx * parts[i].vy - ry * parts[i].vx;
				parts[i].vx = vx;
				parts[i].vy = vy;
			}
			else if(TYP(r) == PT_RSST && !ry && !rx)//if on RSST, make it solid
			{
				int ct_under, tmp_under;

				ct_under = parts[ID(r)].ctype;
				tmp_under = parts[ID(r)].tmp;

				//If there's a correct ctype set, solidify RSST into it
				if(ct_under > 0 && ct_under < PT_NUM)
				{
					sim->create_part(ID(r), x, y, ct_under);

					//If there's a correct tmp set, use it for ctype
					if((tmp_under > 0) && (tmp_under < PT_NUM) && (elements[ct_under].CarriesTypeIn & (1U << FIELD_CTYPE)))
						parts[ID(r)].ctype = tmp_under;
				}
				else
					sim->part_change_type(ID(r), x, y, PT_RSSS); //Default to RSSS if no ctype

				sim->kill_part(i);

				return 1;
			}
			else if (TYP(r) == PT_FILT && parts[ID(r)].tmp==9)
			{
				parts[i].vx += ((float)sim->rng.between(-500, 500))/1000.0f;
				parts[i].vy += ((float)sim->rng.between(-500, 500))/1000.0f;
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int x = 0;
	*colr = *colg = *colb = 0;
	for (x=0; x<12; x++) {
		*colr += (cpart->ctype >> (x+18)) & 1;
		*colb += (cpart->ctype >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		*colg += (cpart->ctype >> (x+9))  & 1;
	x = 624/(*colr+*colg+*colb+1);
	*colr *= x;
	*colg *= x;
	*colb *= x;

	*firea = 100;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;

	*pixel_mode &= ~PMODE_FLAT;
	*pixel_mode |= FIRE_ADD | PMODE_ADD | NO_DECO;
	if (cpart->flags & FLAG_PHOTDECO)
	{
		*pixel_mode &= ~NO_DECO;
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	float a = sim->rng.between(0, 7) * 0.78540f;
	sim->parts[i].vx = 3.0f * cosf(a);
	sim->parts[i].vy = 3.0f * sinf(a);
	if (TYP(sim->pmap[y][x]) == PT_FILT)
		sim->parts[i].ctype = Element_FILT_interactWavelengths(sim, &sim->parts[ID(sim->pmap[y][x])], sim->parts[i].ctype);
}
