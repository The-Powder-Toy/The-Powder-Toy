#include "simulation/ElementCommon.h"

int Element_PIPE_update(UPDATE_FUNC_ARGS);
int Element_PIPE_graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PPIP()
{
	Identifier = "DEFAULT_PT_PPIP";
	Name = "PPIP";
	Colour = PIXPACK(0x444466);
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
	HeatConduct = 0;
	Description = "Powered version of PIPE, use PSCN/NSCN to Activate/Deactivate.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

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

int Element_PPIP_ppip_changed = 0;

void Element_PPIP_flood_trigger(Simulation * sim, int x, int y, int sparkedBy)
{
	int coord_stack_limit = XRES*YRES;
	unsigned short (*coord_stack)[2];
	int coord_stack_size = 0;
	int x1, x2;

	Particle * parts = sim->parts;
	int (*pmap)[XRES] = sim->pmap;

	// Separate flags for on and off in case PPIP is sparked by PSCN and NSCN on the same frame
	// - then PSCN can override NSCN and behaviour is not dependent on particle order
	int prop = 0;
	if (sparkedBy==PT_PSCN) prop = PPIP_TMPFLAG_TRIGGER_ON << 3;
	else if (sparkedBy==PT_NSCN) prop = PPIP_TMPFLAG_TRIGGER_OFF << 3;
	else if (sparkedBy==PT_INST) prop = PPIP_TMPFLAG_TRIGGER_REVERSE << 3;

	if (prop==0 || TYP(pmap[y][x])!=PT_PPIP || (parts[ID(pmap[y][x])].tmp & prop))
		return;

	coord_stack = new unsigned short[coord_stack_limit][2];
	coord_stack[coord_stack_size][0] = x;
	coord_stack[coord_stack_size][1] = y;
	coord_stack_size++;

	do
	{
		coord_stack_size--;
		x = coord_stack[coord_stack_size][0];
		y = coord_stack[coord_stack_size][1];
		x1 = x2 = x;
		// go left as far as possible
		while (x1>=CELL)
		{
			if (TYP(pmap[y][x1-1]) != PT_PPIP)
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2<XRES-CELL)
		{
			if (TYP(pmap[y][x2+1]) != PT_PPIP)
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (!(parts[ID(pmap[y][x])].tmp & prop))
			Element_PPIP_ppip_changed = 1;
			parts[ID(pmap[y][x])].tmp |= prop;
		}

		// add adjacent pixels to stack
		// +-1 to x limits to include diagonally adjacent pixels
		// Don't need to check x bounds here, because already limited to [CELL, XRES-CELL]
		if (y>=CELL+1)
			for (x=x1-1; x<=x2+1; x++)
			if (TYP(pmap[y-1][x]) == PT_PPIP && !(parts[ID(pmap[y-1][x])].tmp & prop))
			{
				coord_stack[coord_stack_size][0] = x;
				coord_stack[coord_stack_size][1] = y-1;
				coord_stack_size++;
				if (coord_stack_size>=coord_stack_limit)
				{
					delete[] coord_stack;
					return;
				}
			}
		if (y<YRES-CELL-1)
			for (x=x1-1; x<=x2+1; x++)
				if (TYP(pmap[y+1][x]) == PT_PPIP && !(parts[ID(pmap[y+1][x])].tmp & prop))
				{
					coord_stack[coord_stack_size][0] = x;
					coord_stack[coord_stack_size][1] = y+1;
					coord_stack_size++;
					if (coord_stack_size>=coord_stack_limit)
					{
						delete[] coord_stack;
						return;
					}
				}
	} while (coord_stack_size>0);
	delete[] coord_stack;
}
