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

signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

//#TPT-Directive ElementHeader Element_PIPE static int update(UPDATE_FUNC_ARGS)
int Element_PIPE::update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np;
	int rnd, rndstore;
	if ((parts[i].tmp&0xFF)>=PT_NUM || !sim->elements[parts[i].tmp&0xFF].Enabled)
		parts[i].tmp &= ~0xFF;
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
						if ((r&0xFF) == PT_BRCK)
						{
							if (parts[i].tmp & PPIP_TMPFLAG_PAUSED)
								parts[r>>8].tmp = 0;
							else
								parts[r>>8].tmp = 1; //make surrounding BRCK glow
						}
					}
				}
		}
		
		if (parts[i].tmp & PPIP_TMPFLAG_TRIGGER_REVERSE)
		{
			parts[i].tmp ^= PPIP_TMPFLAG_REVERSED;
			if (parts[i].ctype == 2) //Switch colors so it goes in reverse
				parts[i].ctype = 4;
			else if (parts[i].ctype == 4)
				parts[i].ctype = 2;
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
	if (parts[i].ctype>=2 && parts[i].ctype<=4 && !(parts[i].tmp & PPIP_TMPFLAG_PAUSED))
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
						if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP)&&parts[r>>8].ctype==1)
						{
							parts[r>>8].ctype = (((parts[i].ctype)%3)+2);//reverse
							parts[r>>8].life = 6;
							if ( parts[i].tmp&0x100)//is a single pixel pipe
							{
								parts[r>>8].tmp |= 0x200;//will transfer to a single pixel pipe
								parts[r>>8].tmp |= count<<10;//coords of where it came from
								parts[i].tmp |= ((7-count)<<14);
								parts[i].tmp |= 0x2000;
							}
							neighborcount ++;
							lastneighbor = r>>8;
						}
						else if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP)&&parts[r>>8].ctype!=(((parts[i].ctype-1)%3)+2))
						{
							neighborcount ++;
							lastneighbor = r>>8;
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
					if (surround_space && !r && (parts[i].tmp&0xFF)!=0)  //creating at end
					{
						np = sim->create_part(-1,x+rx,y+ry,parts[i].tmp&0xFF);
						if (np!=-1)
						{
							transfer_pipe_to_part(sim, parts+i, parts+np);
						}
					}
					//try eating particle at entrance
					else if ((parts[i].tmp&0xFF) == 0 && (sim->elements[r&0xFF].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						if ((r&0xFF)==PT_SOAP)
							Element_SOAP::detach(sim, r>>8);
						transfer_part_to_pipe(parts+(r>>8), parts+i);
						sim->kill_part(r>>8);
					}
					else if ((parts[i].tmp&0xFF) == 0 && (r&0xFF)==PT_STOR && parts[r>>8].tmp>0 && sim->IsValidElement(parts[r>>8].tmp) && (sim->elements[parts[r>>8].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						// STOR stores properties in the same places as PIPE does
						transfer_pipe_to_pipe(parts+(r>>8), parts+i);
					}
				}
			}
		}
	}
	else if (!parts[i].ctype && parts[i].life<=10)
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
						int index = sim->create_part(-1,x+rx,y+ry,PT_BRCK);//BRCK border, people didn't like DMND
						if (parts[i].type == PT_PPIP && index != -1)
							parts[index].tmp = 1;
					}
				}
			}
		if (parts[i].life<=1)
			parts[i].ctype = 1;
	}
	else if (parts[i].ctype==1)//wait for empty space before starting to generate automatic pipe pattern
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
						if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[i].ctype==1 && parts[i].life )
							issingle = 0;
					}
			if (issingle)
				parts[i].tmp |= 0x100;
		}
		else if (parts[i].life==2)
		{
			parts[i].ctype = 2;
			parts[i].life = 6;
		}
	}
	return 0;
}



//#TPT-Directive ElementHeader Element_PIPE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PIPE::graphics(GRAPHICS_FUNC_ARGS)
{
	int t = cpart->tmp & 0xFF;
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
		//*colr = PIXR(elements[cpart->tmp&0xFF].pcolors);
		//*colg = PIXG(elements[cpart->tmp&0xFF].pcolors);
		//*colb = PIXB(elements[cpart->tmp&0xFF].pcolors);
	}
	else
	{
		switch (cpart->ctype){
		case 2:
			*colr = 50;
			*colg = 1;
			*colb = 1;
			break;
		case 3:
			*colr = 1;
			*colg = 50;
			*colb = 1;
			break;
		case 4:
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

//#TPT-Directive ElementHeader Element_PIPE static void transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part)
void Element_PIPE::transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part)
{
	part->type = (pipe->tmp & 0xFF);
	part->temp = pipe->temp;
	part->life = pipe->tmp2;
	part->tmp = pipe->pavg[0];
	part->ctype = pipe->pavg[1];
	pipe->tmp &= ~0xFF;

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
	pipe->tmp = (pipe->tmp&~0xFF) | part->type;
	pipe->temp = part->temp;
	pipe->tmp2 = part->life;
	pipe->pavg[0] = part->tmp;
	pipe->pavg[1] = part->ctype;
}

//#TPT-Directive ElementHeader Element_PIPE static void transfer_pipe_to_pipe(Particle *src, Particle *dest)
void Element_PIPE::transfer_pipe_to_pipe(Particle *src, Particle *dest)
{
	dest->tmp = (dest->tmp&~0xFF) | (src->tmp&0xFF);
	dest->temp = src->temp;
	dest->tmp2 = src->tmp2;
	dest->pavg[0] = src->pavg[0];
	dest->pavg[1] = src->pavg[1];
	src->tmp &= ~0xFF;
}

//#TPT-Directive ElementHeader Element_PIPE static void pushParticle(Simulation * sim, int i, int count, int original)
void Element_PIPE::pushParticle(Simulation * sim, int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q, notctype=(((sim->parts[i].ctype)%3)+2);
	if ((sim->parts[i].tmp&0xFF) == 0 || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
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
				else if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && sim->parts[r>>8].ctype!=notctype && (sim->parts[r>>8].tmp&0xFF)==0)
				{
					transfer_pipe_to_pipe(sim->parts+i, sim->parts+(r>>8));
					if (r>>8 > original)
						sim->parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
					count++;
					pushParticle(sim, r>>8,count,original);
				}
				else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
				{
					int portaltmp = sim->parts[r>>8].tmp;
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
		if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && sim->parts[r>>8].ctype!=notctype && (sim->parts[r>>8].tmp&0xFF)==0)
		{
			transfer_pipe_to_pipe(sim->parts+i, sim->parts+(r>>8));
			if (r>>8 > original)
				sim->parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
			count++;
			pushParticle(sim, r>>8,count,original);
		}
		else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
		{
			int portaltmp = sim->parts[r>>8].tmp;
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
		else if ((r&0xFF) == PT_NONE) //Move particles out of pipe automatically, much faster at ends
		{
			rx = pos_1_rx[coords];
			ry = pos_1_ry[coords];
			np = sim->create_part(-1,x+rx,y+ry,sim->parts[i].tmp&0xFF);
			if (np!=-1)
			{
				transfer_pipe_to_part(sim, sim->parts+i, sim->parts+np);
			}
		}
		
	}
	return;
}


Element_PIPE::~Element_PIPE() {}
