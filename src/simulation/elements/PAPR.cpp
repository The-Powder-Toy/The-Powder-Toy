#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

// Element overview:
// PAPR (Paper) is a flammable solid element that can be colored by certain other elements.
// Additionally, it can be read and written to by ARAY and LDTC.

// Property usage:
// life: Whether or not the particle is marked
// tmp: Temporary read/write state for ARAY interaction
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
	Hardness = 60;

	Weight = 100;

	HeatConduct = 80;
	Description = "Paper. Flammable, can be marked by BCOL or deco. Lets non-solids through when unmarked.";

	Properties = TYPE_SOLID | PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 700.0f;
	HighTemperatureTransition = PT_NONE;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
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

	// Get marked by BCOL
	if (TYP(pmap[y][x]) == PT_BCOL)
	{
		parts[i].life = 1;
		parts[i].dcolour = 0xFF22222A;
	}

	// Doesn't guarantee layering won't happen, but makes it far less likely
	if (TYP(pmap[y][x]) == PT_SAWD)
	{
		parts[ID(pmap[y][x])].tmp = 0;
	}

	// Generally, these should correspond, but correct if they don't.
	if (!parts[i].life != !parts[i].dcolour)
	{
		parts[i].life = parts[i].dcolour ? 1 : 0;
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
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Don't render if there's a particle above you
	int onTopOfMe = gfctx.sim->pmap[ny][nx];
	if (onTopOfMe && &gfctx.sim->parts[ID(onTopOfMe)] != cpart)
	{
		*pixel_mode = PMODE_NONE;
		return 0;
	}
	float burnAmount = std::max((float)cpart->tmp2, cpart->temp);
	if (cpart->life)
	{
		// Render deco color when marked
		if(gfctx.ren->decorations_enable && !gfctx.ren->blackDecorations)
		{
			// Burnt paper has more faded colors
			float alpha = restrict_flt(((cpart->dcolour >> 24) & 0xFF) - restrict_flt((burnAmount - 450) * 1.7f, 0, 255), 0, 255) / 255.f;
			*colr = int(*colr * (1 - alpha) + ((cpart->dcolour >> 16) & 0xFF) * alpha);
			*colg = int(*colg * (1 - alpha) + ((cpart->dcolour >> 8) & 0xFF) * alpha);
			*colb = int(*colb * (1 - alpha) + ((cpart->dcolour) & 0xFF) * alpha);
		}
		else // If deco is disabled or blackDecorations is on, become a generic dark gray color
		{
			float alpha = 1 - restrict_flt((burnAmount - 450) * 1.7f, 0, 255) / 255.f;
			*colr = int(*colr * (1 - alpha) + 20 * alpha);
			*colg = int(*colg * (1 - alpha) + 20 * alpha);
			*colb = int(*colb * (1 - alpha) + 20 * alpha);
		}
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
	*pixel_mode |= NO_DECO;
	return 0;
}
