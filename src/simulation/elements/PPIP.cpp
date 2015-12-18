#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PPIP PT_PPIP 161
Element_PPIP::Element_PPIP()
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

	Temperature = 273.15f;
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

	Update = &Element_PIPE::update;
	Graphics = &Element_PIPE::graphics;
}

#define PFLAG_NORMALSPEED 0x00010000
// parts[].tmp flags
// trigger flags to be processed this frame (trigger flags for next frame are shifted 3 bits to the left):
#define PPIP_TMPFLAG_TRIGGER_ON 0x10000000
#define PPIP_TMPFLAG_TRIGGER_OFF 0x08000000
#define PPIP_TMPFLAG_TRIGGER_REVERSE 0x04000000
#define PPIP_TMPFLAG_TRIGGERS 0x1C000000
// current status of the pipe
#define PPIP_TMPFLAG_PAUSED 0x02000000
#define PPIP_TMPFLAG_REVERSED 0x01000000
// 0x000000FF element
// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction

//#TPT-Directive ElementHeader Element_PPIP static int ppip_changed
int Element_PPIP::ppip_changed = 0;

//#TPT-Directive ElementHeader Element_PPIP static void flood_trigger(Simulation * sim, int x, int y, int sparkedBy)
void Element_PPIP::flood_trigger(Simulation * sim, int x, int y, int sparkedBy)
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

	if (prop==0 || (pmap[y][x]&0xFF)!=PT_PPIP || (parts[pmap[y][x]>>8].tmp & prop))
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
			if ((pmap[y][x1-1]&0xFF)!=PT_PPIP)
			{
				break;
			}
			x1--;
		}
		// go right as far as possible
		while (x2<XRES-CELL)
		{
			if ((pmap[y][x2+1]&0xFF)!=PT_PPIP)
			{
				break;
			}
			x2++;
		}
		// fill span
		for (x=x1; x<=x2; x++)
		{
			if (!(parts[pmap[y][x]>>8].tmp & prop))
			ppip_changed = 1;
			parts[pmap[y][x]>>8].tmp |= prop;
		}

		// add adjacent pixels to stack
		// +-1 to x limits to include diagonally adjacent pixels
		// Don't need to check x bounds here, because already limited to [CELL, XRES-CELL]
		if (y>=CELL+1)
			for (x=x1-1; x<=x2+1; x++)
			if ((pmap[y-1][x]&0xFF)==PT_PPIP && !(parts[pmap[y-1][x]>>8].tmp & prop))
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
				if ((pmap[y+1][x]&0xFF)==PT_PPIP && !(parts[pmap[y+1][x]>>8].tmp & prop))
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

Element_PPIP::~Element_PPIP() {}
