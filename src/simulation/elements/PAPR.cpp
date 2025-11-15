#include "simulation/ElementCommon.h"
#include "PAPR.h"

// Element overview:
// PAPR (Paper) is a flammable solid element that can be colored by certain other elements.
// Additionally, it can be read and written to by ARAY and LDTC.

// Property usage:
// life: Whether or not the particle is marked
// tmp (EPPR): Temporary read/write state for ARAY interaction
// tmp2: Singe level

void Element::Element_PAPR()
{
	Identifier = "DEFAULT_PT_PAPR";
	Name = "PAPR";
	Colour = 0xF3F3CA_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.995f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 80;
	Description = "Paper. Flammable, can be marked by BCOL or erased by SOAP. Lets particles through when unmarked.";

	Properties = TYPE_SOLID | PROP_NEUTPASS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 700.0f;
	HighTemperatureTransition = PT_NONE;

	Update = &Element_PAPR_update;
	Graphics = &Element_PAPR_graphics;
	CtypeDraw = &Element_PAPR_ctypeDraw;
}

int Element_PAPR_update(UPDATE_FUNC_ARGS)
{
	// Char when above burning temperature
	if (parts[i].temp > 450 && parts[i].temp >= parts[i].tmp2)
	{
		parts[i].tmp2 = (int)parts[i].temp;
	}

	// Auto-ignition temperature
	if (parts[i].temp > (451.0f - 32.f) / 1.8f + 273.15f)
	{
		parts[i].temp += 1;
		if (sim->rng.chance((int)parts[i].temp-450,400))
		{
			int np = sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_FIRE);
			if (np >= 0)
			{
				parts[np].life = 70;
			}
		}
	}

	// Electronic marking
	if (parts[i].type == PT_EPPR)
	{
		if (parts[i].tmp3 == 10)
		{
			parts[i].tmp3 = 11;
			parts[i].life = 1;
			parts[i].dcolour = MARK_COLOR_COAL;
		}
		else if (parts[i].tmp3 != 0 && parts[i].tmp3 != 11)
		{
			parts[i].life = 0;
			parts[i].dcolour = 0x00000000;
			parts[i].tmp3--;
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
					if (TYP(r)==PT_SPRK)
					{
						if (parts[ID(r)].ctype == PT_PSCN)
							parts[i].tmp3 = 10;
						else if (parts[ID(r)].ctype == PT_NSCN)
							parts[i].tmp3 = 9;
					}
					else if (TYP(r) == PT_EPPR)
					{
						if (parts[i].tmp3 >= 10 && parts[ID(r)].tmp3 > 0 && parts[ID(r)].tmp3 < 10)
							parts[i].tmp3 = 9;
						else if (parts[i].tmp3 == 0 && parts[ID(r)].tmp3 >= 10)
							parts[i].tmp3 = 10;
					}
				}
			}
		}

		// Decrement tmp counter for laser reading/writing
		if (parts[i].tmp & 0xF)
		{
			parts[i].tmp--;
		}
		else
		{
			parts[i].tmp = 0;
		}
	}

	auto r = pmap[y][x];
	switch (TYP(r))
	{
	// Get marked by BCOL
	case PT_BCOL:
		parts[i].life = 1;
		parts[i].dcolour = MARK_COLOR_COAL;
		break;

	// Acts as a smoke filter
	case PT_SMKE:
		if (parts[i].type == PT_PAPR)
		{
			if (sim->rng.chance(1, 5))
			{
				sim->kill_part(ID(r));
				// On average, each pixel of PAPR absorbs 10 SMKE particles before becoming impermeable
				if (sim->rng.chance(1, 10))
				{
					parts[i].life = 1;
					parts[i].dcolour = 0xFF322222;
				}
			}
		}
		break;

	// Can also filter out CAUS from the air, but much less effectively (partly because of corrosion)
	case PT_CAUS:
		if (parts[i].type == PT_PAPR)
		{
			sim->kill_part(ID(r));
			parts[i].life = 1;
			parts[i].dcolour = 0xFF223C22;
		}
		break;

	// Doesn't guarantee layering won't happen, but makes it far less likely
	case PT_SAWD:
		parts[ID(r)].tmp = 0;
		break;

	case PT_GUNP:
		if (parts[i].type == PT_PAPR)
		{
			if (sim->pv[y / CELL][x / CELL] > 0.75f && sim->rng.chance(1, 5))
			{
				sim->create_part(i, x, y, PT_IGNT);
				parts[i].life = 3;
				sim->kill_part(ID(r));
				return 1;
			}
		}
		break;

	case PT_MERC:
		if (parts[i].type == PT_PAPR && parts[ID(r)].tmp > 0)
		{
			sim->part_change_type(i, x, y, PT_EPPR);
			parts[ID(r)].tmp--;
		}
	}
	return 0;
}

int Element_PAPR_graphics(GRAPHICS_FUNC_ARGS)
{
	// Don't render if there's a particle above you
	int onTopOfMe = gfctx.sim->pmap[ny][nx];
	if (onTopOfMe && &gfctx.sim->parts[ID(onTopOfMe)] != cpart)
	{
		*pixel_mode = PMODE_NONE;
		return 0;
	}
	float burnAmount = std::max((float)cpart->tmp2, cpart->temp);

	// Render deco color with custom blending for burnt paper
	if (gfctx.ren->decorationLevel == RendererSettings::decorationEnabled)
	{
		// Burnt paper has more faded colors
		float alpha = restrict_flt(((cpart->dcolour >> 24) & 0xFF) - restrict_flt((burnAmount - 450) * 1.7f, 0, 255), 0, 255) / 255.f;
		*colr = int(*colr * (1 - alpha) + ((cpart->dcolour >> 16) & 0xFF) * alpha);
		*colg = int(*colg * (1 - alpha) + ((cpart->dcolour >> 8) & 0xFF) * alpha);
		*colb = int(*colb * (1 - alpha) + ((cpart->dcolour) & 0xFF) * alpha);
	}
	else if (cpart->life) // If deco is disabled or blackDecorations is on, marks become a generic dark gray color
	{
		float alpha = 1 - restrict_flt((burnAmount - 450) * 1.7f, 0, 255) / 255.f;
		*colr = int(*colr * (1 - alpha) + 20 * alpha);
		*colg = int(*colg * (1 - alpha) + 20 * alpha);
		*colb = int(*colb * (1 - alpha) + 20 * alpha);
	}

	// Darken when burnt
	if (burnAmount > 450)
	{
		*colr -= (int)restrict_flt((burnAmount - 450) * 1.2f, 0, 220);
		*colg -= (int)restrict_flt((burnAmount - 450) * 1.4f, 0, 230);
		*colb -= (int)restrict_flt((burnAmount - 450) * 1.6f, 0, 197);
	}
	if (cpart->tmp)
	{
		*pixel_mode |= PMODE_GLOW;
		float flash = (cpart->tmp & 0xF) / 3.f;
		*cola = int(flash * 200);
		*colr = int(*colr * (1 - flash));
		*colg = int(*colg * (1 - flash));
		*colb = int(*colb * (1 - flash));
		if (cpart->tmp & 0x10)
		{
			*colr += int(255 * flash);
		}
		else
		{
			*colg += int(255 * flash);
			*colb += int(255 * flash);
		}
	}
	// Disable deco since we're doing it ourselves
	*pixel_mode |= NO_DECO;
	return 0;
}

bool Element_PAPR_ctypeDraw(CTYPEDRAW_FUNC_ARGS)
{
	// Allow "drawing" directly on PAPR like a pencil
	if (t == PT_BCOL || t == PT_COAL)
	{
		sim->parts[i].life = 1;
		sim->parts[i].dcolour = MARK_COLOR_COAL;
	}
	// Similarly, erase with SOAP
	if (t == PT_SOAP)
	{
		sim->parts[i].life = 0;
		sim->parts[i].dcolour = 0x00000000;
	}
	return false;
}
