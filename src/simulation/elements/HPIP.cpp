#include "simulation/ElementCommon.h"
#include "PIPE.h"

void Element::Element_HPIP()
{
	Identifier = "DEFAULT_PT_HPIP";
	Name = "HPIP";
	Colour = 0x444466_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	DefaultProperties.temp = 273.15f;
	HeatConduct = 251;
	Description = "No description.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 60;

	Update = &Element_PIPE_update;
	Graphics = &Element_PIPE_graphics;
}

// parts[].tmp flags
// trigger flags to be processed this frame (trigger flags for next frame are shifted 3 bits to the left):
constexpr int PPIP_TMPFLAG_TRIGGER_ON      = 0x10000000;
constexpr int PPIP_TMPFLAG_TRIGGER_OFF     = 0x08000000;
constexpr int PPIP_TMPFLAG_TRIGGER_REVERSE = 0x04000000;
// 0x000000FF element
// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction
