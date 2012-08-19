#include "simulation/Elements.h"

Element::Element():
	Identifier("DEFAULT_INVALID"),
	Name(""),
	Colour(PIXPACK(0xFF00FF)),
	MenuVisible(0),
	MenuSection(0),
	Enabled(0),

	Advection(0.0f),
	AirDrag(-0.0f * CFDS),
	AirLoss(1.0f),
	Loss(1.0f),
	Collision(0.0f),
	Gravity(0.0f),
	Diffusion(0.0f),
	HotAir(0.0f * CFDS),
	Falldown(0),

	Flammable(0),
	Explosive(0),
	Meltable(0),
	Hardness(30),

	Weight(50),

	Temperature(273.15f),
	Enthalpy(0.0f),
	HeatConduct(128),
	Description("No description"),

	State(ST_SOLID),
	Properties(TYPE_SOLID),

	LowPressure(IPL),
	LowPressureTransition(NT),
	HighPressure(IPH),
	HighPressureTransition(NT),
	LowTemperature(ITL),
	LowTemperatureTransition(NT),
	HighTemperature(ITH),
	HighTemperatureTransition(NT),

	Update(NULL),
	Graphics(&Element::defaultGraphics),
	IconGenerator(NULL)
{
}

int Element::defaultGraphics(GRAPHICS_FUNC_ARGS)
{
	int t = cpart->type;
	//Property based defaults
	if(ren->sim->elements[t].Properties & PROP_RADIOACTIVE) *pixel_mode |= PMODE_GLOW;
	if(ren->sim->elements[t].Properties & TYPE_LIQUID)
	{
		*pixel_mode |= PMODE_BLUR;
	}
	if(ren->sim->elements[t].Properties & TYPE_GAS)
	{
		*pixel_mode &= ~PMODE;
		*pixel_mode |= FIRE_BLEND;
		*firer = *colr/2;
		*fireg = *colg/2;
		*fireb = *colb/2;
		*firea = 125;
		*pixel_mode |= DECO_FIRE;
	}
	return 1;
}
