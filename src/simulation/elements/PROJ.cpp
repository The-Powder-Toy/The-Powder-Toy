#include "simulation/ElementCommon.h"
static int update(UPDATE_FUNC_ARGS);
static bool ctypeDraw(CTYPEDRAW_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);

void Element::Element_PROJ()
{
	Identifier = "DEFAULT_PT_PROJ";
	Name = "PROJ";
	Colour = PIXPACK(0xFFE21D24);
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.00f;
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

	HeatConduct = 0;
	Description = "Projectile, converts into ctype upon collision. SPRK with PSCN to launch. Use .tmp for range and temp. for power.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	DefaultProperties.temp = R_TEMP + 30.0f; //Defualt power.

	Update = &update;
	CtypeDraw = &ctypeDraw;
	Graphics = &graphics;
	Create = &create;
}

static int update(UPDATE_FUNC_ARGS)
{
	//checks for .tmp and .temp
	if (parts[i].tmp <= 0 || parts[i].tmp > 500)
		parts[i].tmp = 10;
	if (parts[i].temp <= 273.15f || parts[i].temp > 473.15f )
		parts[i].temp = 293.15f;

	for (int rx = -1; rx <= 1; rx++)
		for (int ry = -1; ry <= 1; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				int r = pmap[y + ry][x + rx];
				if (!r)
					continue;
				if(parts[ID(r)].type == PT_SPRK && parts[ID(r)].ctype == PT_PSCN && parts[ID(r)].life == 3) //Check for a sprk with ctype PSCN to activate and store the direction.
				{
					parts[i].pavg[1] = -rx;
					parts[i].pavg[2] = -ry;
					parts[i].life = 10;
				}
				//For collision detection.
				if (parts[i].life == 10 && parts[ID(r)].type != PT_EMBR && parts[ID(r)].type != PT_SPRK && parts[ID(r)].type != PT_PSCN && (sim->elements[TYP(r)].Properties & TYPE_SOLID || sim->elements[TYP(r)].Properties & TYPE_PART || sim->elements[TYP(r)].Properties & TYPE_LIQUID))
				{
					sim->part_change_type(i, x, y, parts[i].ctype);
				}
			}
	if (parts[i].life == 10) //For motion, .temp determines the power while .tmp deterimines the range. 
	{
		parts[i].tmp2+= 1;
		parts[i].vx = parts[i].pavg[1]*((parts[i].temp-273.15f)/10);
		parts[i].vy = parts[i].pavg[2] + 0.2*(parts[i].tmp2/parts[i].tmp);
		sim->create_part(-1, x, y - 1, PT_EMBR);
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS) //Flare when activated.
{
	if(cpart->life == 10)
	*pixel_mode |= PMODE_LFLARE;
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS) //Default range and ctype settings.
{
	sim->parts[i].tmp = 10;
	sim->parts[i].ctype = PT_BOMB;
}
static bool ctypeDraw(CTYPEDRAW_FUNC_ARGS) //For enabling ctype Draw.
{
	if (!Element::ctypeDrawVInCtype(CTYPEDRAW_FUNC_SUBCALL_ARGS))
	{
		return false;
	}
	if (t == PT_LIGH)
	{
		sim->parts[i].ctype |= PMAPID(30);
	}
	return true;
}