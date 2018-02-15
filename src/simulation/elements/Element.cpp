#include "simulation/Elements.h"
#include "simulation/StructProperty.h"

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
	PhotonReflectWavelengths(0x3FFFFFFF),

	Weight(50),

	Temperature(273.15f),
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
	IconGenerator(nullptr)
{
}

std::vector<StructProperty> Element::GetProperties()
{
	std::vector<StructProperty> properties;
	properties.emplace_back("Name",							StructProperty::String,		offsetof(Element, Name));
	properties.emplace_back("Colour",						StructProperty::Colour,		offsetof(Element, Colour));
	properties.emplace_back("Color",						StructProperty::Colour,		offsetof(Element, Colour));
	properties.emplace_back("MenuVisible",					StructProperty::Integer,	offsetof(Element, MenuVisible));
	properties.emplace_back("MenuSection",					StructProperty::Integer,	offsetof(Element, MenuSection));
	properties.emplace_back("Advection",					StructProperty::Float,		offsetof(Element, Advection));
	properties.emplace_back("AirDrag",						StructProperty::Float,		offsetof(Element, AirDrag));
	properties.emplace_back("AirLoss",						StructProperty::Float,		offsetof(Element, AirLoss));
	properties.emplace_back("Loss",							StructProperty::Float,		offsetof(Element, Loss));
	properties.emplace_back("Collision",					StructProperty::Float,		offsetof(Element, Collision));
	properties.emplace_back("Gravity",						StructProperty::Float,		offsetof(Element, Gravity));
	properties.emplace_back("Diffusion",					StructProperty::Float,		offsetof(Element, Diffusion));
	properties.emplace_back("HotAir",						StructProperty::Float,		offsetof(Element, HotAir));
	properties.emplace_back("Falldown",						StructProperty::Integer,	offsetof(Element, Falldown));
	properties.emplace_back("Flammable",					StructProperty::Integer,	offsetof(Element, Flammable));
	properties.emplace_back("Explosive",					StructProperty::Integer,	offsetof(Element, Explosive));
	properties.emplace_back("Meltable",						StructProperty::Integer,	offsetof(Element, Meltable));
	properties.emplace_back("Hardness",						StructProperty::Integer,	offsetof(Element, Hardness));
	properties.emplace_back("PhotonReflectWavelengths",		StructProperty::UInteger,	offsetof(Element, PhotonReflectWavelengths));
	properties.emplace_back("Weight",						StructProperty::Integer,	offsetof(Element, Weight));
	properties.emplace_back("Temperature",					StructProperty::Float,		offsetof(Element, Temperature));
	properties.emplace_back("HeatConduct",					StructProperty::UChar,		offsetof(Element, HeatConduct));
	properties.emplace_back("Description",					StructProperty::String,		offsetof(Element, Description));
	properties.emplace_back("State",						StructProperty::Removed,	0);
	properties.emplace_back("Properties",					StructProperty::Integer,	offsetof(Element, Properties));
	properties.emplace_back("LowPressure",					StructProperty::Float,		offsetof(Element, LowPressure));
	properties.emplace_back("LowPressureTransition",		StructProperty::Integer,	offsetof(Element, LowPressureTransition));
	properties.emplace_back("HighPressure",					StructProperty::Float,		offsetof(Element, HighPressure));
	properties.emplace_back("HighPressureTransition",		StructProperty::Integer,	offsetof(Element, HighPressureTransition));
	properties.emplace_back("LowTemperature",				StructProperty::Float,		offsetof(Element, LowTemperature));
	properties.emplace_back("LowTemperatureTransition",		StructProperty::Integer,	offsetof(Element, LowTemperatureTransition));
	properties.emplace_back("HighTemperature",				StructProperty::Float,		offsetof(Element, HighTemperature));
	properties.emplace_back("HighTemperatureTransition",	StructProperty::Integer,	offsetof(Element, HighTemperatureTransition));
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
					if ((TYP(r)==PT_WATR||TYP(r)==PT_DSTW||TYP(r)==PT_SLTW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_WATR);
					}
					if ((TYP(r)==PT_ICEI || TYP(r)==PT_SNOW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						if (1>(rand()%1000))
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
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && 1>(rand()%10))
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
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && 1>(rand()%10))
					{
						if (rand()%4==0) sim->part_change_type(i,x,y,PT_SALT);
						else sim->part_change_type(i,x,y,PT_WTRV);
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
					if ((TYP(r)==PT_FIRE || TYP(r)==PT_LAVA) && 1>(rand()%10))
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
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && 1>(rand()%1000))
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
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_ICEI);
						sim->part_change_type(ID(r),x+rx,y+ry,PT_ICEI);
					}
					if ((TYP(r)==PT_WATR || TYP(r)==PT_DSTW) && 15>(rand()%1000))
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
		parts[i].life = rand()%50+120;
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
