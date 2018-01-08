#include "simulation/Elements.h"
//Temp particle used for graphics
Particle tpart;

//#TPT-Directive ElementClass Element_PIPE PT_PIPE 99
Element_PIPE::Element_PIPE()
{
	Identifier = "DEFAULT_PT_PIPE";
	Name = "PIPE";
	Colour = PIXPACK(0x444444);
	MenuVisible = 1;
	MenuSection = SC_FORCE;
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
	Description = "PIPE, moves particles around. Once the BRCK generates, erase some for the exit. Then the PIPE generates and is usable.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 10.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PIPE::update;
	Graphics = &Element_PIPE::graphics;

	memset(&tpart, 0, sizeof(Particle));
}

// 0x000000FF element
// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction
// 0x000E0000 PIPE color data stored here

#define PFLAG_NORMALSPEED            0x00010000
#define PFLAG_INITIALIZING           0x00020000 // colors haven't been set yet
#define PFLAG_COLOR_RED              0x00040000
#define PFLAG_COLOR_GREEN            0x00080000
#define PFLAG_COLOR_BLUE             0x000C0000
#define PFLAG_COLORS                 0x000C0000

#define PPIP_TMPFLAG_REVERSED        0x01000000
#define PPIP_TMPFLAG_PAUSED          0x02000000
#define PPIP_TMPFLAG_TRIGGER_REVERSE 0x04000000
#define PPIP_TMPFLAG_TRIGGER_OFF     0x08000000
#define PPIP_TMPFLAG_TRIGGER_ON      0x10000000
#define PPIP_TMPFLAG_TRIGGERS        0x1C000000

signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

//#TPT-Directive ElementHeader Element_PIPE static int update(UPDATE_FUNC_ARGS)
int Element_PIPE::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np;
	int rnd, rndstore;
	if (parts[i].ctype && !sim->elements[TYP(parts[i].ctype)].Enabled)
		parts[i].ctype = 0;
	if (parts[i].tmp & PPIP_TMPFLAG_TRIGGERS)
	{
		int pause_changed = 0;
		if (parts[i].tmp & PPIP_TMPFLAG_TRIGGER_ON) // TRIGGER_ON overrides TRIGGER_OFF
		{
			if (parts[i].tmp & PPIP_TMPFLAG_PAUSED)
				pause_changed = 1;
			parts[i].tmp &= ~PPIP_TMPFLAG_PAUSED;
		}
		else if (parts[i].tmp & PPIP_TMPFLAG_TRIGGER_OFF)
		{
			if (!(parts[i].tmp & PPIP_TMPFLAG_PAUSED))
				pause_changed = 1;
			parts[i].tmp |= PPIP_TMPFLAG_PAUSED;
		}
		if (pause_changed)
		{
			int rx, ry, r;
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
				{
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (TYP(r) == PT_BRCK)
						{
							if (parts[i].tmp & PPIP_TMPFLAG_PAUSED)
								parts[ID(r)].tmp = 0;
							else
								parts[ID(r)].tmp = 1; //make surrounding BRCK glow
						}
					}
				}
		}
	
		if (parts[i].tmp & PPIP_TMPFLAG_TRIGGER_REVERSE)
		{
			parts[i].tmp ^= PPIP_TMPFLAG_REVERSED;
			// Switch colors so it goes in reverse
			if ((parts[i].tmp&PFLAG_COLORS) != PFLAG_COLOR_GREEN)
				parts[i].tmp ^= PFLAG_COLOR_GREEN;
			if (parts[i].tmp & 0x100) //Switch one pixel pipe direction
			{
				int coords = (parts[i].tmp>>13)&0xF;
				int coords2 = (parts[i].tmp>>9)&0xF;
				parts[i].tmp &= ~0x1FE00;
				parts[i].tmp |= coords<<9;
				parts[i].tmp |= coords2<<13;
			}
		}
		
		parts[i].tmp &= ~PPIP_TMPFLAG_TRIGGERS;
	}
	if ((parts[i].tmp&PFLAG_COLORS) && !(parts[i].tmp & PPIP_TMPFLAG_PAUSED))
	{
		if (parts[i].life==3)
		{
			int lastneighbor = -1;
			int neighborcount = 0;
			int count = 0;
			// make automatic pipe pattern
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (TYP(r) != PT_PIPE && TYP(r) != PT_PPIP)
							continue;
						unsigned int nextColor = (((((parts[i].tmp&PFLAG_COLORS)>>18)+1)%3)+1)<<18;
						if (parts[ID(r)].tmp&PFLAG_INITIALIZING)
						{
							parts[ID(r)].tmp |= nextColor;
							parts[ID(r)].tmp &= ~PFLAG_INITIALIZING;
							parts[ID(r)].life = 6;
							if (parts[i].tmp&0x100)//is a single pixel pipe
							{
								parts[ID(r)].tmp |= 0x200;//will transfer to a single pixel pipe
								parts[ID(r)].tmp |= count<<10;//coords of where it came from
								parts[i].tmp |= ((7-count)<<14);
								parts[i].tmp |= 0x2000;
							}
							neighborcount ++;
							lastneighbor = ID(r);
						}
						else if ((parts[ID(r)].tmp&PFLAG_COLORS) != nextColor)
						{
							neighborcount ++;
							lastneighbor = ID(r);
						}
						count++;
					}
			if (neighborcount == 1)
				parts[lastneighbor].tmp |= 0x100;
		}
		else
		{
			if (parts[i].flags&PFLAG_NORMALSPEED)//skip particle push to prevent particle number being higher causing speed up
			{
				parts[i].flags &= ~PFLAG_NORMALSPEED;
			}
			else
			{
				pushParticle(sim, i,0,i);
			}

			if (nt)//there is something besides PIPE around current particle
			{
				rndstore = rand();
				rnd = rndstore&7;
				//rndstore = rndstore>>3;
				rx = pos_1_rx[rnd];
				ry = pos_1_ry[rnd];
				if (BOUNDS_CHECK)
				{
					r = pmap[y+ry][x+rx];
					if(!r)
						r = sim->photons[y+ry][x+rx];
					if (surround_space && !r && TYP(parts[i].ctype))  //creating at end
					{
						np = sim->create_part(-1, x+rx, y+ry, TYP(parts[i].ctype));
						if (np!=-1)
						{
							transfer_pipe_to_part(sim, parts+i, parts+np);
						}
					}
					//try eating particle at entrance
					else if (!TYP(parts[i].ctype) && (sim->elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						if (TYP(r)==PT_SOAP)
							Element_SOAP::detach(sim, ID(r));
						transfer_part_to_pipe(parts+(ID(r)), parts+i);
						sim->kill_part(ID(r));
					}
					else if (!TYP(parts[i].ctype) && TYP(r)==PT_STOR && parts[ID(r)].tmp>0 && sim->IsValidElement(parts[ID(r)].tmp) && (sim->elements[parts[ID(r)].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						// STOR stores properties in the same places as PIPE does
						transfer_pipe_to_pipe(parts+(ID(r)), parts+i, true);
					}
				}
			}
		}
	}
	else if (!(parts[i].tmp&(PFLAG_COLORS|PFLAG_INITIALIZING)) && parts[i].life<=10)
	{
		// make a border
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
			{
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
					{
						// BRCK border
						int index = sim->create_part(-1,x+rx,y+ry,PT_BRCK);
						if (parts[i].type == PT_PPIP && index != -1)
							parts[index].tmp = 1;
					}
				}
			}
		if (parts[i].life <= 1)
			parts[i].tmp |= PFLAG_INITIALIZING;
	}
	// Wait for empty space before starting to generate automatic pipe pattern
	else if (parts[i].tmp & PFLAG_INITIALIZING)
	{
		if (!parts[i].life)
		{
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						if (!pmap[y+ry][x+rx] && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_ALLOWAIR && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALL && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALLELEC && (sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_EWALL || sim->emap[(y+ry)/CELL][(x+rx)/CELL]))
							parts[i].life=50;
					}
		}
		else if (parts[i].life==5)//check for beginning of pipe single pixel
		{
			int issingle = 1;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && parts[i].life)
							issingle = 0;
					}
			if (issingle)
				parts[i].tmp |= 0x100;
		}
		else if (parts[i].life == 2)
		{
			parts[i].tmp |= PFLAG_COLOR_RED;
			parts[i].tmp &= ~PFLAG_INITIALIZING;
			parts[i].life = 6;
		}
	}
	return 0;
}



//#TPT-Directive ElementHeader Element_PIPE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PIPE::graphics(GRAPHICS_FUNC_ARGS)
{
	int t = TYP(cpart->ctype);
	if (t>0 && t<PT_NUM && ren->sim->elements[t].Enabled)
	{
		if (t == PT_STKM || t == PT_STKM2 || t == PT_FIGH)
			return 0;
		if (ren->graphicscache[t].isready)
		{
			*pixel_mode = ren->graphicscache[t].pixel_mode;
			*cola = ren->graphicscache[t].cola;
			*colr = ren->graphicscache[t].colr;
			*colg = ren->graphicscache[t].colg;
			*colb = ren->graphicscache[t].colb;
			*firea = ren->graphicscache[t].firea;
			*firer = ren->graphicscache[t].firer;
			*fireg = ren->graphicscache[t].fireg;
			*fireb = ren->graphicscache[t].fireb;
		}
		else
		{
			//Emulate the graphics of stored particle
			tpart.type = t;
			tpart.temp = cpart->temp;
			tpart.life = cpart->tmp2;
			tpart.tmp = cpart->pavg[0];
			tpart.ctype = cpart->pavg[1];
			if (t == PT_PHOT && tpart.ctype == 0x40000000)
				tpart.ctype = 0x3FFFFFFF;

			*colr = PIXR(ren->sim->elements[t].Colour);
			*colg = PIXG(ren->sim->elements[t].Colour);
			*colb = PIXB(ren->sim->elements[t].Colour);
			if (ren->sim->elements[t].Graphics)
			{
				(*(ren->sim->elements[t].Graphics))(ren, &tpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
			}
			else
			{
				Element::defaultGraphics(ren, &tpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
			}
		}
		//*colr = PIXR(elements[t].pcolors);
		//*colg = PIXG(elements[t].pcolors);
		//*colb = PIXB(elements[t].pcolors);
	}
	else
	{
		switch (cpart->tmp & PFLAG_COLORS)
		{
		case PFLAG_COLOR_RED:
			*colr = 50;
			*colg = 1;
			*colb = 1;
			break;
		case PFLAG_COLOR_GREEN:
			*colr = 1;
			*colg = 50;
			*colb = 1;
			break;
		case PFLAG_COLOR_BLUE:
			*colr = 1;
			*colg = 1;
			*colb = 50;
			break;
		default:
			break;
		}
	}
	return 0;
}

//#TPT-Directive ElementHeader Element_PIPE static void transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR=false)
void Element_PIPE::transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR)
{
	// STOR also calls this function to move particles from STOR to PRTI
	// PIPE was changed, so now PIPE and STOR don't use the same particle storage format
	if (STOR)
	{
		part->type = TYP(pipe->tmp);
		pipe->tmp = 0;
	}
	else
	{
		part->type = TYP(pipe->ctype);
		pipe->ctype = 0;
	}
	part->temp = pipe->temp;
	part->life = pipe->tmp2;
	part->tmp = pipe->pavg[0];
	part->ctype = pipe->pavg[1];

	if (!(sim->elements[part->type].Properties & TYPE_ENERGY))
	{
		part->vx = 0.0f;
		part->vy = 0.0f;
	}
	else if (part->type == PT_PHOT && part->ctype == 0x40000000)
		part->ctype = 0x3FFFFFFF;
	part->tmp2 = 0;
	part->flags = 0;
	part->dcolour = 0;
}

//#TPT-Directive ElementHeader Element_PIPE static void transfer_part_to_pipe(Particle *part, Particle *pipe)
void Element_PIPE::transfer_part_to_pipe(Particle *part, Particle *pipe)
{
	pipe->ctype = part->type;
	pipe->temp = part->temp;
	pipe->tmp2 = part->life;
	pipe->pavg[0] = part->tmp;
	pipe->pavg[1] = part->ctype;
}

//#TPT-Directive ElementHeader Element_PIPE static void transfer_pipe_to_pipe(Particle *src, Particle *dest, bool STOR=false)
void Element_PIPE::transfer_pipe_to_pipe(Particle *src, Particle *dest, bool STOR)
{
	// STOR to PIPE
	if (STOR)
	{
		dest->ctype = src->tmp;
		src->tmp = 0;
	}
	else
	{
		dest->ctype = src->ctype;	
		src->ctype = 0;
	}
	dest->temp = src->temp;
	dest->tmp2 = src->tmp2;
	dest->pavg[0] = src->pavg[0];
	dest->pavg[1] = src->pavg[1];
}

//#TPT-Directive ElementHeader Element_PIPE static void pushParticle(Simulation * sim, int i, int count, int original)
void Element_PIPE::pushParticle(Simulation * sim, int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q;
	unsigned int notctype = (((((sim->parts[i].tmp&PFLAG_COLORS)>>18)+1)%3)+1)<<18;
	if (!TYP(sim->parts[i].ctype) || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
		return;
	x = (int)(sim->parts[i].x+0.5f);
	y = (int)(sim->parts[i].y+0.5f);
	if( !(sim->parts[i].tmp&0x200) )
	{ 
		//normal random push
		rndstore = rand();
		// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
		// so can go 5 cycles without regenerating rndstore
		for (q=0; q<3; q++)//try to push 3 times
		{
			rnd = rndstore&7;
			rndstore = rndstore>>3;
			rx = pos_1_rx[rnd];
			ry = pos_1_ry[rnd];
			if (BOUNDS_CHECK)
			{
				r = sim->pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
				{
					transfer_pipe_to_pipe(sim->parts+i, sim->parts+(ID(r)));
					if (ID(r) > original)
						sim->parts[ID(r)].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
					count++;
					pushParticle(sim, ID(r),count,original);
				}
				else if (TYP(r) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
				{
					int portaltmp = sim->parts[ID(r)].tmp;
					if (portaltmp >= CHANNELS)
						portaltmp = CHANNELS-1;
					else if (portaltmp < 0)
						portaltmp = 0;
					for (int nnx = 0; nnx < 80; nnx++)
						if (!sim->portalp[portaltmp][count][nnx].type)
						{
							transfer_pipe_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]));
							count++;
							break;
						}
				}
			}
		}
	}
	else //predefined 1 pixel thick pipe movement
	{
		int coords = 7 - ((sim->parts[i].tmp>>10)&7);
		r = sim->pmap[y+ pos_1_ry[coords]][x+ pos_1_rx[coords]];
		if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
		{
			transfer_pipe_to_pipe(sim->parts+i, sim->parts+(ID(r)));
			if (ID(r) > original)
				sim->parts[ID(r)].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
			count++;
			pushParticle(sim, ID(r),count,original);
		}
		else if (TYP(r) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
		{
			int portaltmp = sim->parts[ID(r)].tmp;
			if (portaltmp >= CHANNELS)
				portaltmp = CHANNELS-1;
			else if (portaltmp < 0)
				portaltmp = 0;
			for (int nnx = 0; nnx < 80; nnx++)
				if (!sim->portalp[portaltmp][count][nnx].type)
				{
					transfer_pipe_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]));
					count++;
					break;
				}
		}
		else if (!r) //Move particles out of pipe automatically, much faster at ends
		{
			rx = pos_1_rx[coords];
			ry = pos_1_ry[coords];
			np = sim->create_part(-1,x+rx,y+ry,TYP(sim->parts[i].ctype));
			if (np!=-1)
			{
				transfer_pipe_to_part(sim, sim->parts+i, sim->parts+np);
			}
		}
		
	}
	return;
}


Element_PIPE::~Element_PIPE() {}
