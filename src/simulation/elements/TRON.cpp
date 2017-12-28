#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_TRON PT_TRON 143
Element_TRON::Element_TRON()
{
	Identifier = "DEFAULT_PT_TRON";
	Name = "TRON";
	Colour = PIXPACK(0xA9FF00);
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

	Temperature = 0.0f;
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

	Update = &Element_TRON::update;
	Graphics = &Element_TRON::graphics;

	Element_TRON::init_graphics();
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

#define TRON_HEAD 1
#define TRON_NOGROW 2
#define TRON_WAIT 4 //it was just created, so WAIT a frame
#define TRON_NODIE 8
#define TRON_DEATH 16 //Crashed, now dying
#define TRON_NORANDOM 65536
int tron_rx[4] = {-1, 0, 1, 0};
int tron_ry[4] = { 0,-1, 0, 1};
unsigned int tron_colours[32];

//#TPT-Directive ElementHeader Element_TRON static void init_graphics()
void Element_TRON::init_graphics()
{
	int i;
	int r, g, b;
	for (i=0; i<32; i++)
	{
		HSV_to_RGB(i<<4,255,255,&r,&g,&b);
		tron_colours[i] = r<<16 | g<<8 | b;
	}
}

//#TPT-Directive ElementHeader Element_TRON static int update(UPDATE_FUNC_ARGS)
int Element_TRON::update(UPDATE_FUNC_ARGS)
{
	if (parts[i].tmp&TRON_WAIT)
	{
		parts[i].tmp &= ~TRON_WAIT;
		return 0;
	}
	if (parts[i].tmp&TRON_HEAD)
	{
		int firstdircheck = 0,seconddir,seconddircheck = 0,lastdir,lastdircheck = 0;
		int direction = (parts[i].tmp>>5 & 0x3);
		int originaldir = direction;

		//random turn
		int random = rand()%340;
		if ((random==1 || random==3) && !(parts[i].tmp & TRON_NORANDOM))
		{
			//randomly turn left(3) or right(1)
			direction = (direction + random)%4;
		}

		//check in front
		//do sight check
		firstdircheck = Element_TRON::trymovetron(sim,x,y,direction,i,parts[i].tmp2);
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
				seconddir = (direction + ((rand()%2)*2)+1)% 4;
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
		if (Element_TRON::new_tronhead(sim,x + tron_rx[direction],y + tron_ry[direction],i,direction) == -1)
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



//#TPT-Directive ElementHeader Element_TRON static int graphics(GRAPHICS_FUNC_ARGS)
int Element_TRON::graphics(GRAPHICS_FUNC_ARGS)
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

//#TPT-Directive ElementHeader Element_TRON static int new_tronhead(Simulation * sim, int x, int y, int i, int direction)
int Element_TRON::new_tronhead(Simulation * sim, int x, int y, int i, int direction)
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

//#TPT-Directive ElementHeader Element_TRON static int trymovetron(Simulation * sim, int x, int y, int dir, int i, int len)
int Element_TRON::trymovetron(Simulation * sim, int x, int y, int dir, int i, int len)
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
		if (canmovetron(sim, r, k-1) && !sim->bmap[(ry)/CELL][(rx)/CELL] && ry > CELL && rx > CELL && ry < YRES-CELL && rx < XRES-CELL)
		{
			count++;
			for (tx = rx - tron_ry[dir] , ty = ry - tron_rx[dir], j=1; abs(tx-rx) < (len-k) && abs(ty-ry) < (len-k); tx-=tron_ry[dir],ty-=tron_rx[dir],j++)
			{
				r = sim->pmap[ty][tx];
				if (canmovetron(sim, r, j+k-1) && !sim->bmap[(ty)/CELL][(tx)/CELL] && ty > CELL && tx > CELL && ty < YRES-CELL && tx < XRES-CELL)
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
				if (canmovetron(sim, r, j+k-1) && !sim->bmap[(ty)/CELL][(tx)/CELL] && ty > CELL && tx > CELL && ty < YRES-CELL && tx < XRES-CELL)
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

//#TPT-Directive ElementHeader Element_TRON static bool canmovetron(Simulation * sim, int r, int len)
bool Element_TRON::canmovetron(Simulation * sim, int r, int len)
{
	if (!r || (TYP(r) == PT_SWCH && sim->parts[ID(r)].life >= 10) || (TYP(r) == PT_INVIS && sim->parts[ID(r)].tmp2 == 1))
		return true;
	if ((((sim->elements[TYP(r)].Properties & PROP_LIFE_KILL_DEC) && sim->parts[ID(r)].life > 0)|| ((sim->elements[TYP(r)].Properties & PROP_LIFE_KILL) && (sim->elements[TYP(r)].Properties & PROP_LIFE_DEC))) && sim->parts[ID(r)].life < len)
		return true;
	return false;
}

Element_TRON::~Element_TRON() {}
