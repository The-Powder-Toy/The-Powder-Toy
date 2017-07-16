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

	Update(NULL),
	Graphics(&Element::defaultGraphics),
	IconGenerator(NULL)
{
}

std::vector<StructProperty> Element::GetProperties()
{
	std::vector<StructProperty> properties;
	properties.push_back(StructProperty("Name",							StructProperty::String,		offsetof(Element, Name)));
	properties.push_back(StructProperty("Colour",						StructProperty::Colour,		offsetof(Element, Colour)));
	properties.push_back(StructProperty("Color",						StructProperty::Colour,		offsetof(Element, Colour)));
	properties.push_back(StructProperty("MenuVisible",					StructProperty::Integer,	offsetof(Element, MenuVisible)));
	properties.push_back(StructProperty("MenuSection",					StructProperty::Integer,	offsetof(Element, MenuSection)));
	properties.push_back(StructProperty("Advection",					StructProperty::Float,		offsetof(Element, Advection)));
	properties.push_back(StructProperty("AirDrag",						StructProperty::Float,		offsetof(Element, AirDrag)));
	properties.push_back(StructProperty("AirLoss",						StructProperty::Float,		offsetof(Element, AirLoss)));
	properties.push_back(StructProperty("Loss",							StructProperty::Float,		offsetof(Element, Loss)));
	properties.push_back(StructProperty("Collision",					StructProperty::Float,		offsetof(Element, Collision)));
	properties.push_back(StructProperty("Gravity",						StructProperty::Float,		offsetof(Element, Gravity)));
	properties.push_back(StructProperty("Diffusion",					StructProperty::Float,		offsetof(Element, Diffusion)));
	properties.push_back(StructProperty("HotAir",						StructProperty::Float,		offsetof(Element, HotAir)));
	properties.push_back(StructProperty("Falldown",						StructProperty::Integer,	offsetof(Element, Falldown)));
	properties.push_back(StructProperty("Flammable",					StructProperty::Integer,	offsetof(Element, Flammable)));
	properties.push_back(StructProperty("Explosive",					StructProperty::Integer,	offsetof(Element, Explosive)));
	properties.push_back(StructProperty("Meltable",						StructProperty::Integer,	offsetof(Element, Meltable)));
	properties.push_back(StructProperty("Hardness",						StructProperty::Integer,	offsetof(Element, Hardness)));
	properties.push_back(StructProperty("PhotonReflectWavelengths",		StructProperty::UInteger,	offsetof(Element, PhotonReflectWavelengths)));
	properties.push_back(StructProperty("Weight",						StructProperty::Integer,	offsetof(Element, Weight)));
	properties.push_back(StructProperty("Temperature",					StructProperty::Float,		offsetof(Element, Temperature)));
	properties.push_back(StructProperty("HeatConduct",					StructProperty::UChar,		offsetof(Element, HeatConduct)));
	properties.push_back(StructProperty("Description",					StructProperty::String,		offsetof(Element, Description)));
	properties.push_back(StructProperty("State",						StructProperty::Removed,	0));
	properties.push_back(StructProperty("Properties",					StructProperty::Integer,	offsetof(Element, Properties)));
	properties.push_back(StructProperty("LowPressure",					StructProperty::Float,		offsetof(Element, LowPressure)));
	properties.push_back(StructProperty("LowPressureTransition",		StructProperty::Integer,	offsetof(Element, LowPressureTransition)));
	properties.push_back(StructProperty("HighPressure",					StructProperty::Float,		offsetof(Element, HighPressure)));
	properties.push_back(StructProperty("HighPressureTransition",		StructProperty::Integer,	offsetof(Element, HighPressureTransition)));
	properties.push_back(StructProperty("LowTemperature",				StructProperty::Float,		offsetof(Element, LowTemperature)));
	properties.push_back(StructProperty("LowTemperatureTransition",		StructProperty::Integer,	offsetof(Element, LowTemperatureTransition)));
	properties.push_back(StructProperty("HighTemperature",				StructProperty::Float,		offsetof(Element, HighTemperature)));
	properties.push_back(StructProperty("HighTemperatureTransition",	StructProperty::Integer,	offsetof(Element, HighTemperatureTransition)));
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
					if (((r&0xFF)==PT_WATR||(r&0xFF)==PT_DSTW||(r&0xFF)==PT_SLTW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						sim->part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					}
					if (((r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_WATR);
						if (1>(rand()%1000))
							sim->part_change_type(r>>8,x+rx,y+ry,PT_WATR);
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
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
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
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
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
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
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
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_ICEI);
						sim->part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
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
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
					{
						sim->part_change_type(i,x,y,PT_ICEI);
						sim->part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
					}
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 15>(rand()%1000))
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
