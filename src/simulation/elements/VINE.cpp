#include "simulation/ElementCommon.h"
#include "PLNT.h"
#include <algorithm>
#include <array>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_VINE()
{
	Identifier = "DEFAULT_PT_VINE";
	Name = "VINE";
	Colour = 0x079A00_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;

	Weight = 100;

	HeatConduct = 65;
	Description = "Vine, can grow along WOOD.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE; //@ VINE -> FIRE

	DefaultProperties.tmp = 1;

	Update = &update;
	Graphics = &graphics; // this used to be missing, maybe for a reason?
}

// Replicate PLNT leaf color table locally so VINE can render identical leaf colors when it has PLNT ctype
constexpr std::array<std::array<int, 3>, 8> leafColor = { {
	{{ 243, 246, 244 }}, // White
	{{ 255, 223, 50  }}, // Yellow
	{{ 255, 183, 197 }}, // Pink
	{{ 250, 0,   25  }}, // Red
	{{ 128, 206, 196 }}, // Cyan / light blue
	{{ 127, 255, 0   }}, // Bright green
	{{ 0,   74,  178 }}, // Blue
	{{ 12,  172, 0   }}  // Usual PLNT green
} };

static int update(UPDATE_FUNC_ARGS)
{
	int rndstore = sim->rng.gen();
	auto rx = (rndstore % 3) - 1;
	rndstore >>= 2;
	auto ry = (rndstore % 3) - 1;
	rndstore >>= 2;
	if (rx || ry)
	{
		auto r = pmap[y + ry][x + rx];
		//@ VINE -> PLNT
		if (!(rndstore % 15))
		{
			// convert this VINE to PLNT, preserve PLNT colour bits from VINE's ctype
			sim->part_change_type(i, x, y, PT_PLNT);
			// Ensure resulting PLNT has the colour stored in PLNT_COLOUR bits and set dir to 7
			parts[i].ctype = (parts[i].ctype & (0x3f << PLNT_COLOUR)) | (7 << PLNT_DIR);
		}
		else if (!r)
		{
			auto np = sim->create_part(-1, x + rx, y + ry, PT_VINE);
			if (np < 0) return 0;
			parts[np].temp = parts[i].temp;
			// propagate ctype and some state to the new VINE so color/genome is preserved
			parts[np].ctype = parts[i].ctype;
			parts[np].tmp = parts[i].tmp;
			parts[np].tmp2 = parts[i].tmp2;

			// convert the original VINE into PLNT, preserve colour
			sim->part_change_type(i, x, y, PT_PLNT);
			parts[i].ctype = (parts[i].ctype & (0x3f << PLNT_COLOUR)) | (7 << PLNT_DIR);
		}
	}
	if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
		parts[i].tmp2 = (int)parts[i].temp;
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// If this VINE carries PLNT colour bits, render like PLNT leaf colours.
	int colour = (cpart->ctype >> PLNT_COLOUR) & 0x3f;
	if (colour != 0)
	{
		int cyan = (colour & 0b110000) != 0;
		int magenta = (colour & 0b001100) != 0;
		int yellow = (colour & 0b000011) != 0;

		int ind = 4 * cyan + 2 * magenta + yellow;
		if (0 <= ind && ind < 8)
		{
			*colr = leafColor[ind][0];
			*colg = leafColor[ind][1];
			*colb = leafColor[ind][2];
			return 0;
		}
	}

	// Otherwise use base VINE rendering with temperature tinting (same as original behavior)
	float maxtemp = std::max((float)cpart->tmp2, cpart->temp);
	if (maxtemp > 300)
	{
		*colr += (int)restrict_flt((maxtemp - 300) / 5, 0, 58);
		*colg -= (int)restrict_flt((maxtemp - 300) / 2, 0, 102);
		*colb += (int)restrict_flt((maxtemp - 300) / 5, 0, 70);
	}
	if (maxtemp < 273)
	{
		*colg += (int)restrict_flt((273 - maxtemp) / 4, 0, 255);
		*colb += (int)restrict_flt((273 - maxtemp) / 1.5, 0, 255);
	}
	return 0;
}
