#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
static void create(ELEMENT_CREATE_FUNC_ARGS);
static int trymovetron(Simulation * sim, int x, int y, int dir, int i, int len);
static bool canmovetron(Simulation * sim, int r, int len);
static int new_tronhead(Simulation * sim, int x, int y, int i, int direction);

void Element::Element_TRON()
{
	Identifier = "DEFAULT_PT_TRON";
	Name = "TRON";
	Colour = 0xA9FF00_rgb;
	MenuVisible = 1;
	MenuSection = SC_SPECIAL;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f  * CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	DefaultProperties.temp = 0.0f;
	HeatConduct = 40;
	Description = "Smart particles, Travels in straight lines and avoids obstacles. Grows with time.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC|PROP_LIFE_KILL;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
	Create = &create;
}

/* TRON element is meant to resemble a tron bike (or worm) moving around and trying to avoid obstacles itself.
 * It has four direction each turn to choose from, 0 (left) 1 (up) 2 (right) 3 (down).
 * Each turn has a small random chance to randomly turn one way (so it doesn't do the exact same thing in a large room)
 * If the place it wants to move isn't a barrier, it will try and 'see' in front of itself to determine its safety.
 * For now the tron can only see its own body length in pixels ahead of itself (and around corners)
 *  - - - - - - - - - -
 *  - - - - + - - - - -
 *  - - - + + + - - - -
 *  - - +<--+-->+ - - -
 *  - +<----+---->+ - -
 *  - - - - H - - - - -
 * Where H is the head with tail length 4, it checks the + area to see if it can hit any of the edges, then it is called safe, or picks the biggest area if none safe.
 * .tmp bit values: 1st head, 2nd no tail growth, 3rd wait flag, 4th Nodie, 5th Dying, 6th & 7th is direction, 8th - 16th hue, 17th Norandom
 * .tmp2 is tail length (gets longer every few hundred frames)
 * .life is the timer that kills the end of the tail (the head uses life for how often it grows longer)
 * .ctype Contains the colour, lost on save, regenerated using hue tmp (bits 7 - 16)
 */

constexpr auto TRON_HEAD     = UINT32_C(0x00000001);
constexpr auto TRON_NOGROW   = UINT32_C(0x00000002);
constexpr auto TRON_WAIT     = UINT32_C(0x00000004); //it was just created, so WAIT a frame
constexpr auto TRON_NODIE    = UINT32_C(0x00000008);
constexpr auto TRON_DEATH    = UINT32_C(0x00000010); //Crashed, now dying
constexpr auto TRON_NORANDOM = UINT32_C(0x00010000);
constexpr int tron_rx[4] = {-1, 0, 1, 0};
constexpr int tron_ry[4] = { 0,-1, 0, 1};

static const std::array<unsigned int, 32> MakeTronColors()
{
	std::array<unsigned int, 32> tron_colours;
	int i;
	int r, g, b;
	for (i=0; i<32; i++)
	{
		// funny almost-bug: if (i<<4) > 360(ish), HSV_to_RGB does nothing with r/g/b,
		// but since the variables are reused across iterations of the loop, they will still have sane values
		HSV_to_RGB(i<<4,255,255,&r,&g,&b);
		tron_colours[i] = r<<16 | g<<8 | b;
	}
	return tron_colours;
}
static const auto tron_colours = MakeTronColors();

static int update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp&TRON_WAIT)
	{
		parts[i].tmp &= ~TRON_WAIT;
		return 0;
	}
	if (parts[i].tmp&TRON_HEAD)
	{
		int firstdircheck = 0, seconddir = 0, seconddircheck = 0, lastdir = 0, lastdircheck = 0;
		int direction = (parts[i].tmp>>5 & 0x3);
		int originaldir = direction;

		//random turn
		int random = sim->rng.between(0, 339);
		if ((random==1 || random==3) && !(parts[i].tmp & TRON_NORANDOM))
		{
			//randomly turn left(3) or right(1)
			direction = (direction + random)%4;
		}

		//check in front
		//do sight check
		firstdircheck = trymovetron(sim,x,y,direction,i,parts[i].tmp2);
		if (firstdircheck < parts[i].tmp2)
		{
			if (parts[i].tmp & TRON_NORANDOM)
			{
				seconddir = (direction + 1)%4;
				lastdir = (direction + 3)%4;
			}
			else if (originaldir != direction) //if we just tried a random turn, don't pick random again
			{
				seconddir = originaldir;
				lastdir = (direction + 2)%4;
			}
			else
			{
				seconddir = (direction + (sim->rng.between(0, 1)*2)+1)% 4;
				lastdir = (seconddir + 2)%4;
			}
			seconddircheck = trymovetron(sim,x,y,seconddir,i,parts[i].tmp2);
			lastdircheck = trymovetron(sim,x,y,lastdir,i,parts[i].tmp2);
		}
		//find the best move
		if (seconddircheck > firstdircheck)
			direction = seconddir;
		if (lastdircheck > seconddircheck && lastdircheck > firstdircheck)
			direction = lastdir;
		//now try making new head, even if it fails
		if (new_tronhead(sim,x + tron_rx[direction],y + tron_ry[direction],i,direction) == -1)
		{
			//ohgod crash
			parts[i].tmp |= TRON_DEATH;
			//trigger tail death for TRON_NODIE, or is that mode even needed? just set a high tail length(but it still won't start dying when it crashes)
		}

		//set own life and clear .tmp (it dies if it can't move anyway)
		parts[i].life = parts[i].tmp2;
		parts[i].tmp &= parts[i].tmp&0xF818;
	}
	else // fade tail deco, or prevent tail from dying
	{
		if (parts[i].tmp&TRON_NODIE)
			parts[i].life++;
		//parts[i].dcolour =  clamp_flt((float)parts[i].life/(float)parts[i].tmp2,0,1.0f) << 24 |  parts[i].dcolour&0x00FFFFFF;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	unsigned int col = tron_colours[(cpart->tmp&0xF800)>>11];
	if(cpart->tmp & TRON_HEAD)
		*pixel_mode |= PMODE_GLOW;
	*colr = (col & 0xFF0000)>>16;
	*colg = (col & 0x00FF00)>>8;
	*colb = (col & 0x0000FF);
	if(cpart->tmp & TRON_DEATH)
	{
		*pixel_mode |= FIRE_ADD | PMODE_FLARE;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
		*firea = 255;
	}
	if(cpart->life < cpart->tmp2 && !(cpart->tmp & TRON_HEAD))
	{
		*pixel_mode |= PMODE_BLEND;
		*pixel_mode &= ~PMODE_FLAT;
		*cola = (int)((((float)cpart->life)/((float)cpart->tmp2))*255.0f);
	}
	return 0;
}

static void create(ELEMENT_CREATE_FUNC_ARGS)
{
	int randhue = sim->rng.between(0, 359);
	int randomdir = sim->rng.between(0, 3);
	// Set as a head and a direction
	sim->parts[i].tmp = 1 | (randomdir << 5) | (randhue << 7);
	// Tail
	sim->parts[i].tmp2 = 4;
	sim->parts[i].life = 5;
}

static int new_tronhead(Simulation * sim, int x, int y, int i, int direction)
{
	int np = sim->create_part(-1, x , y ,PT_TRON);
	if (np==-1)
		return -1;
	if (sim->parts[i].life >= 100) // increase tail length
	{
		if (!(sim->parts[i].tmp&TRON_NOGROW))
			sim->parts[i].tmp2++;
		sim->parts[i].life = 5;
	}
	//give new head our properties
	sim->parts[np].tmp = 1 | direction<<5 | (sim->parts[i].tmp&(TRON_NOGROW|TRON_NODIE|TRON_NORANDOM)) | (sim->parts[i].tmp&0xF800);
	if (np > i)
		sim->parts[np].tmp |= TRON_WAIT;

	sim->parts[np].ctype = sim->parts[i].ctype;
	sim->parts[np].tmp2 = sim->parts[i].tmp2;
	sim->parts[np].life = sim->parts[i].life + 2;
	return 1;
}

static int trymovetron(Simulation * sim, int x, int y, int dir, int i, int len)
{
	int k,j,r,rx,ry,tx,ty,count;
	count = 0;
	rx = x;
	ry = y;
	for (k = 1; k <= len; k ++)
	{
		rx += tron_rx[dir];
		ry += tron_ry[dir];
		r = sim->pmap[ry][rx];
		if (canmovetron(sim, r, k-1) && !sim->bmap[(ry)/CELL][(rx)/CELL] && ry >= CELL && rx >= CELL && ry < YRES-CELL && rx < XRES-CELL)
		{
			count++;
			for (tx = rx - tron_ry[dir] , ty = ry - tron_rx[dir], j=1; abs(tx-rx) < (len-k) && abs(ty-ry) < (len-k); tx-=tron_ry[dir],ty-=tron_rx[dir],j++)
			{
				r = sim->pmap[ty][tx];
				if (canmovetron(sim, r, j+k-1) && !sim->bmap[(ty)/CELL][(tx)/CELL] && ty >= CELL && tx >= CELL && ty < YRES-CELL && tx < XRES-CELL)
				{
					if (j == (len-k))//there is a safe path, so we can break out
						return len+1;
					count++;
				}
				else //we hit a block so no need to check farther here
					break;
			}
			for (tx = rx + tron_ry[dir] , ty = ry + tron_rx[dir], j=1; abs(tx-rx) < (len-k) && abs(ty-ry) < (len-k); tx+=tron_ry[dir],ty+=tron_rx[dir],j++)
			{
				r = sim->pmap[ty][tx];
				if (canmovetron(sim, r, j+k-1) && !sim->bmap[(ty)/CELL][(tx)/CELL] && ty >= CELL && tx >= CELL && ty < YRES-CELL && tx < XRES-CELL)
				{
					if (j == (len-k))
						return len+1;
					count++;
				}
				else
					break;
			}
		}
		else //a block infront, no need to continue
			break;
	}
	return count;
}

static bool canmovetron(Simulation * sim, int r, int len)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (!r || (TYP(r) == PT_SWCH && sim->parts[ID(r)].life >= 10) || (TYP(r) == PT_INVIS && sim->parts[ID(r)].tmp2 == 1))
		return true;
	if ((((elements[TYP(r)].Properties & PROP_LIFE_KILL_DEC) && sim->parts[ID(r)].life > 0)|| ((elements[TYP(r)].Properties & PROP_LIFE_KILL) && (elements[TYP(r)].Properties & PROP_LIFE_DEC))) && sim->parts[ID(r)].life < len)
		return true;
	return false;
}
