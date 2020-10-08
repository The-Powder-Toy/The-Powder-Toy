#include "ElementCommon.h"
#include "StructProperty.h"

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
	NewtonianGravity(1.0f),
	Diffusion(0.0f),
	HotAir(0.0f * CFDS),
	Falldown(0),

	Flammable(0),
	Explosive(0),
	Meltable(0),
	Hardness(30),
	PhotonReflectWavelengths(0x3FFFFFFF),

	Weight(50),

	HeatConduct(128),
	Description("No description"),

	Properties(TYPE_SOLID),

	LowPressure(IPL),
	LowPressureTransition(NT),
	HighPressure(IPH),
	HighPressureTransition(NT),
	LowTemperature(ITL),
	LowTemperatureTransition(NT),
	HighTemperature(ITH),
	HighTemperatureTransition(NT),

	Update(nullptr),
	Graphics(&Element::defaultGraphics),
	CtypeDraw(nullptr),
	IconGenerator(nullptr)
{
	memset(&DefaultProperties, 0, sizeof(Particle));
	DefaultProperties.temp = R_TEMP + 273.15f;
}

std::vector<StructProperty> const &Element::GetProperties()
{
	static std::vector<StructProperty> properties = {
		{ "Name",                      StructProperty::String,   offsetof(Element, Name                     ) },
		{ "Colour",                    StructProperty::Colour,   offsetof(Element, Colour                   ) },
		{ "Color",                     StructProperty::Colour,   offsetof(Element, Colour                   ) },
		{ "MenuVisible",               StructProperty::Integer,  offsetof(Element, MenuVisible              ) },
		{ "MenuSection",               StructProperty::Integer,  offsetof(Element, MenuSection              ) },
		{ "Enabled",                   StructProperty::Integer,  offsetof(Element, Enabled                  ) },
		{ "Advection",                 StructProperty::Float,    offsetof(Element, Advection                ) },
		{ "AirDrag",                   StructProperty::Float,    offsetof(Element, AirDrag                  ) },
		{ "AirLoss",                   StructProperty::Float,    offsetof(Element, AirLoss                  ) },
		{ "Loss",                      StructProperty::Float,    offsetof(Element, Loss                     ) },
		{ "Collision",                 StructProperty::Float,    offsetof(Element, Collision                ) },
		{ "Gravity",                   StructProperty::Float,    offsetof(Element, Gravity                  ) },
		{ "NewtonianGravity",          StructProperty::Float,    offsetof(Element, NewtonianGravity         ) },
		{ "Diffusion",                 StructProperty::Float,    offsetof(Element, Diffusion                ) },
		{ "HotAir",                    StructProperty::Float,    offsetof(Element, HotAir                   ) },
		{ "Falldown",                  StructProperty::Integer,  offsetof(Element, Falldown                 ) },
		{ "Flammable",                 StructProperty::Integer,  offsetof(Element, Flammable                ) },
		{ "Explosive",                 StructProperty::Integer,  offsetof(Element, Explosive                ) },
		{ "Meltable",                  StructProperty::Integer,  offsetof(Element, Meltable                 ) },
		{ "Hardness",                  StructProperty::Integer,  offsetof(Element, Hardness                 ) },
		{ "PhotonReflectWavelengths",  StructProperty::UInteger, offsetof(Element, PhotonReflectWavelengths ) },
		{ "Weight",                    StructProperty::Integer,  offsetof(Element, Weight                   ) },
		{ "Temperature",               StructProperty::Float,    offsetof(Element, DefaultProperties.temp   ) },
		{ "HeatConduct",               StructProperty::UChar,    offsetof(Element, HeatConduct              ) },
		{ "Description",               StructProperty::String,   offsetof(Element, Description              ) },
		{ "State",                     StructProperty::Removed,  0                                            },
		{ "Properties",                StructProperty::Integer,  offsetof(Element, Properties               ) },
		{ "LowPressure",               StructProperty::Float,    offsetof(Element, LowPressure              ) },
		{ "LowPressureTransition",     StructProperty::TransitionType,  offsetof(Element, LowPressureTransition    ) },
		{ "HighPressure",              StructProperty::Float,    offsetof(Element, HighPressure             ) },
		{ "HighPressureTransition",    StructProperty::TransitionType,  offsetof(Element, HighPressureTransition   ) },
		{ "LowTemperature",            StructProperty::Float,    offsetof(Element, LowTemperature           ) },
		{ "LowTemperatureTransition",  StructProperty::TransitionType,  offsetof(Element, LowTemperatureTransition ) },
		{ "HighTemperature",           StructProperty::Float,    offsetof(Element, HighTemperature          ) },
		{ "HighTemperatureTransition", StructProperty::TransitionType,  offsetof(Element, HighTemperatureTransition) }
	};
	return properties;
}

int Element::legacyUpdate(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int t = parts[i].type;
	if (t==PT_WTRV) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_WATR||TYP(r)==PT_DSTW||TYP(r)==PT_SLTW) && RNG::Ref().chance(1, 1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_WATR);
					}
					if ((TYP(r)==PT_ICEI || TYP(r)==PT_SNOW) && RNG::Ref().chance(1, 1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						if (RNG::Ref().chance(1, 1000))
							sim->part_change_type(ID(r),x+rx,y+ry,PT_WATR);
					}
				}
	}
	else if (t==PT_WATR) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && RNG::Ref().chance(1, 10))
					{
						sim->part_change_type(i,x,y,PT_WTRV);
					}
				}
	}
	else if (t==PT_SLTW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && RNG::Ref().chance(1, 10))
					{
						if (RNG::Ref().chance(1, 4))
							sim->part_change_type(i,x,y,PT_SALT);
						else
							sim->part_change_type(i,x,y,PT_WTRV);
					}
				}
	}
	else if (t==PT_DSTW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && RNG::Ref().chance(1, 10))
					{
						sim->part_change_type(i,x,y,PT_WTRV);
					}
				}
	}
	else if (t==PT_ICEI) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && RNG::Ref().chance(1, 1000))
					{
						sim->part_change_type(i,x,y,PT_ICEI);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_ICEI);
					}
				}
	}
	else if (t==PT_SNOW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && RNG::Ref().chance(1, 1000))
					{
						sim->part_change_type(i,x,y,PT_ICEI);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_ICEI);
					}
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && RNG::Ref().chance(3, 200))
						sim->part_change_type(i,x,y,PT_WATR);
				}
	}
	if (t==PT_WTRV && sim->pv[y/CELL][x/CELL]>4.0f)
		sim->part_change_type(i,x,y,PT_DSTW);
	if (t==PT_OIL && sim->pv[y/CELL][x/CELL]<-6.0f)
		sim->part_change_type(i,x,y,PT_GAS);
	if (t==PT_GAS && sim->pv[y/CELL][x/CELL]>6.0f)
		sim->part_change_type(i,x,y,PT_OIL);
	if (t==PT_DESL && sim->pv[y/CELL][x/CELL]>12.0f)
	{
		sim->part_change_type(i,x,y,PT_FIRE);
		parts[i].life = RNG::Ref().between(120, 169);
	}
	return 0;
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

bool Element::basicCtypeDraw(CTYPEDRAW_FUNC_ARGS)
{
	if (sim->parts[i].type == t || sim->elements[t].Properties & PROP_NOCTYPEDRAW)
	{
		return false;
	}
	sim->parts[i].ctype = t;
	return true;
}

bool Element::ctypeDrawVInTmp(CTYPEDRAW_FUNC_ARGS)
{
	if (!Element::basicCtypeDraw(CTYPEDRAW_FUNC_SUBCALL_ARGS))
	{
		return false;
	}
	if (t == PT_LIFE)
	{
		sim->parts[i].tmp = v;
	}
	return true;
}

bool Element::ctypeDrawVInCtype(CTYPEDRAW_FUNC_ARGS)
{
	if (!Element::basicCtypeDraw(CTYPEDRAW_FUNC_SUBCALL_ARGS))
	{
		return false;
	}
	if (t == PT_LIFE)
	{
		sim->parts[i].ctype |= PMAPID(v);
	}
	return true;
}
