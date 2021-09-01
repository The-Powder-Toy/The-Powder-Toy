#include "simulation/ElementCommon.h"
//Temp particle used for graphics
Particle btpart;

int Element_BVES_update(UPDATE_FUNC_ARGS);
int Element_BVES_graphics(GRAPHICS_FUNC_ARGS);
void Element_BVES_transfer_bves_to_part(Simulation * sim, Particle *bves, Particle *part, bool STOR);
static void transfer_part_to_bves(Particle *part, Particle *bves);
static void transfer_bves_to_bves(Particle *src, Particle *dest, bool STOR);
static void pushParticle(Simulation * sim, int i, int count, int original);
void Element_SOAP_detach(Simulation * sim, int i);

void Element::Element_BVES()
{
	Identifier = "DEFAULT_PT_BVES";
	Name = "BVES";
	Colour = PIXPACK(0x400000);
	MenuVisible = 1;
	MenuSection = SC_BIO;
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
	Description = "Blood vessel. Use like PIPE, it will carry blood and distribute oxygen.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC|TYPE_BIO;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 10.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 60;

	Update = &Element_BVES_update;
	Graphics = &Element_BVES_graphics;

	memset(&btpart, 0, sizeof(Particle));
}

// 0x000000FF element
// 0x00000100 is single pixel bves
// 0x00000200 will transfer like a single pixel bves when in forward mode
// 0x00001C00 forward single pixel bves direction
// 0x00002000 will transfer like a single pixel bves when in reverse mode
// 0x0001C000 reverse single pixel bves direction
// 0x000E0000 BVES color data stored here

constexpr int PFLAG_NORMALSPEED            = 0x00010000;
constexpr int PFLAG_INITIALIZING           = 0x00020000; // colors haven't been set yet
constexpr int PFLAG_COLOR_RED              = 0x00040000;
constexpr int PFLAG_COLOR_GREEN            = 0x00080000;
constexpr int PFLAG_COLOR_BLUE             = 0x000C0000;
constexpr int PFLAG_COLORS                 = 0x000C0000;

constexpr int PPIP_TMPFLAG_REVERSED        = 0x01000000;
constexpr int PPIP_TMPFLAG_PAUSED          = 0x02000000;
constexpr int PPIP_TMPFLAG_TRIGGER_REVERSE = 0x04000000;
constexpr int PPIP_TMPFLAG_TRIGGER_OFF     = 0x08000000;
constexpr int PPIP_TMPFLAG_TRIGGER_ON      = 0x10000000;
constexpr int PPIP_TMPFLAG_TRIGGERS        = 0x1C000000;

signed char bpos_1_rx[] = { -1,-1,-1, 0, 0, 1, 1, 1 };
signed char bpos_1_ry[] = { -1, 0, 1,-1, 1,-1, 0, 1 };
int bpos_1_patch90[] = { 2, 4, 7, 1, 6, 0, 3, 5 };

void Element_BVES_patch90(Particle &part)
{
	auto oldDirForward = (part.tmp & 0x00001C00) >> 10;
	auto newDirForward = bpos_1_patch90[oldDirForward];
	auto oldDirReverse = (part.tmp & 0x0001C000) >> 14;
	auto newDirReverse = bpos_1_patch90[oldDirReverse];
	part.tmp = (part.tmp & 0xFFFE23FF) | (newDirForward << 10) | (newDirReverse << 14);
}

static unsigned int prevColor(unsigned int flags)
{
	unsigned int color = flags & PFLAG_COLORS;
	if (color == PFLAG_COLOR_RED)
		return PFLAG_COLOR_GREEN;
	else if (color == PFLAG_COLOR_GREEN)
		return PFLAG_COLOR_BLUE;
	else if (color == PFLAG_COLOR_BLUE)
		return PFLAG_COLOR_RED;
	return PFLAG_COLOR_RED;
}

static unsigned int nextColor(unsigned int flags)
{
	unsigned int color = flags & PFLAG_COLORS;
	if (color == PFLAG_COLOR_RED)
		return PFLAG_COLOR_BLUE;
	else if (color == PFLAG_COLOR_BLUE)
		color = PFLAG_COLOR_GREEN;
	else if (color == PFLAG_COLOR_GREEN)
		return PFLAG_COLOR_RED;
	return PFLAG_COLOR_GREEN;
}

int Element_BVES_update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, np;
	int rnd, rndstore;

    /*
    // O2 use
    if (RNG::Ref().chance(1, 300)){

		if (parts[i].pavg[0] > 0){
        	parts[i].pavg[0] -= 1;
			parts[i].pavg[1] += 1;
		}
    }

    // Take O2 from blood within pipe
    if (parts[i].ctype == PT_BLD){
        if (parts[i].pavg[0] < MAX_O2 && parts[i].tmp4 > 0){
            parts[i].pavg[0] += 1;
            parts[i].tmp4 -= 1;
        }
    }

    rx =  RNG::Ref().between(-2, 2);
    ry =  RNG::Ref().between(-2, 2);

    // Diffuse among biological
    if (BOUNDS_CHECK && (rx || ry))
    {
        r = pmap[y+ry][x+rx];
        int ir = ID(r);

        if (r) {
            if (sim->elements[TYP(r)].Properties & TYPE_BIO){
                if (parts[i].pavg[0] > parts[ir].tmp){
                    parts[i].pavg[0].tmp -= 1;
                    parts[ir].tmp++;
                }
                if (parts[i].tmp2 > parts[ir].tmp2){
                    parts[i].tmp2--;
                    parts[ir].tmp2++;
                }
            }
        }
    }
    

    if (RNG::Ref().chance(1, 50)){
        // Health stuff
        if (parts[i].tmp5 > 0 && parts[i].pavg[0] < 1){
            parts[i].tmp5 -= 1;
        }
        else{
            if (parts[i].tmp5 < 100){
				parts[i].tmp5++;
			}
        }
    }

    if (parts[i].tmp5 < 1){
        sim->part_change_type(i, x, y, PT_DT);
    }
    */

   if (parts[i].ctype == PT_BLD ||  parts[i].ctype == PT_NONE){

	   if (RNG::Ref().chance(1, 20)){

			rx =  RNG::Ref().between(-2, 2);
			ry =  RNG::Ref().between(-2, 2);
			
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (r){
			
					int tr = TYP(r);
					int ir = ID(r);

					if (sim->elements[tr].Properties & TYPE_BIO && tr != PT_BVES){
						if (parts[i].bio.o2 > 0 && parts[ir].bio.o2 < MAX_O2){
							parts[i].bio.o2--;
							parts[ir].bio.o2++;
						}

						if (parts[i].bio.co2 < MAX_CO2 && parts[ir].bio.co2 > 0){
							parts[i].bio.co2++;
							parts[ir].bio.co2--;
						}
					}
					else if (sim->elements[tr].Properties & TYPE_BIO){
						if (parts[i].bio.o2 > parts[ir].bio.o2){
							parts[i].bio.o2--;
							parts[ir].bio.o2++;
						}
						if (parts[i].bio.co2 > parts[ir].bio.co2){
							parts[i].bio.co2--;
							parts[ir].bio.co2++;
						}
					} 
				}
			}
	   }
                    
   }
   //else if (parts[i].type == PT_NONE){
       // Die if the wall breaks
       //sim->part_change_type(i, x, y, PT_DT);
   //}

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
			if (parts[i].tmp & 0x100) //Switch one pixel bves direction
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
			// make automatic bves pattern
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						count++;
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (TYP(r) != PT_BVES && TYP(r) != PT_PPIP)
							continue;
						unsigned int next = nextColor(parts[i].tmp);
						unsigned int prev = prevColor(parts[i].tmp);
						if (parts[ID(r)].tmp&PFLAG_INITIALIZING)
						{
							parts[ID(r)].tmp |= next;
							parts[ID(r)].tmp &= ~PFLAG_INITIALIZING;
							parts[ID(r)].life = 6;
							// Is a single pixel bves
							if (parts[i].tmp&0x100)
							{
								// Will transfer to a single pixel bves
								parts[ID(r)].tmp |= 0x200;
								// Coords of where it came from
								parts[ID(r)].tmp |= (count - 1) << 10;
								parts[i].tmp |= (8 - count) << 14;
								parts[i].tmp |= 0x2000;
							}
							neighborcount ++;
							lastneighbor = ID(r);
						}
						else if ((parts[ID(r)].tmp&PFLAG_COLORS) != prev)
						{
							neighborcount ++;
							lastneighbor = ID(r);
						}
					}
			if (neighborcount == 1)
				parts[lastneighbor].tmp |= 0x100;
		}
		else
		{
   			//for (int z = 0; z < 2; z++){

			if (parts[i].flags&PFLAG_NORMALSPEED)//skip particle push to prevent particle number being higher causing speed up
			{
				parts[i].flags &= ~PFLAG_NORMALSPEED;
			}
			else
			{
				pushParticle(sim, i,0,i);
			}

			if (nt)//there is something besides BVES around current particle
			{
				rndstore = RNG::Ref().gen();
				rnd = rndstore&7;
				//rndstore = rndstore>>3;
				rx = bpos_1_rx[rnd];
				ry = bpos_1_ry[rnd];
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
							Element_BVES_transfer_bves_to_part(sim, parts+i, parts+np, false);
						}
					}
					//try eating particle at entrance
					else if (!TYP(parts[i].ctype) && (sim->elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						if (TYP(r)==PT_SOAP)
							Element_SOAP_detach(sim, ID(r));
						transfer_part_to_bves(parts+(ID(r)), parts+i);
						sim->kill_part(ID(r));
					}
					else if (!TYP(parts[i].ctype) && TYP(r)==PT_STOR && sim->IsElement(parts[ID(r)].tmp) && (sim->elements[parts[ID(r)].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						// STOR stores properties in the same places as BVES does
						transfer_bves_to_bves(parts+(ID(r)), parts+i, true);
					}
				}
			}
		//}
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
						int index = sim->create_part(-1,x+rx,y+ry,PT_MEAT);
					}
				}
			}
		if (parts[i].life <= 1)
			parts[i].tmp |= PFLAG_INITIALIZING;
	}
	// Wait for empty space before starting to generate automatic bves pattern
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
		else if (parts[i].life==5)//check for beginning of bves single pixel
		{
			int issingle = 1;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (BOUNDS_CHECK && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((TYP(r)==PT_BVES || TYP(r) == PT_PPIP) && parts[i].life)
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

int Element_BVES_graphics(GRAPHICS_FUNC_ARGS)
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
			btpart.type = t;
			btpart.temp = cpart->temp;
			btpart.life = cpart->tmp2;
			btpart.tmp = int(cpart->pavg[0]);
			btpart.ctype = int(cpart->pavg[1]);
			if (t == PT_PHOT && btpart.ctype == 0x40000000)
				btpart.ctype = 0x3FFFFFFF;

			*colr = PIXR(ren->sim->elements[t].Colour);
			*colg = PIXG(ren->sim->elements[t].Colour);
			*colb = PIXB(ren->sim->elements[t].Colour);
			if (ren->sim->elements[t].Graphics)
			{
				(*(ren->sim->elements[t].Graphics))(ren, &btpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
			}
			else
			{
				Element::defaultGraphics(ren, &btpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
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
			*colr = 35;
			*colg = 0;
			*colb = 0;
			break;
		case PFLAG_COLOR_GREEN:
			*colr = 55;
			*colg = 0;
			*colb = 0;
			break;
		case PFLAG_COLOR_BLUE:
			*colr = 20;
			*colg = 0;
			*colb = 0;
			break;
		default:
			break;
		}
	}
	return 0;
}

void Element_BVES_transfer_bves_to_part(Simulation * sim, Particle *bves, Particle *part, bool STOR)
{
	// STOR also calls this function to move particles from STOR to PRTI
	// PIPE was changed, so now PIPE and STOR don't use the same particle storage format
	if (STOR)
	{
		part->type = TYP(bves->tmp);
		bves->tmp = 0;
	}
	else
	{
		part->type = TYP(bves->ctype);
		bves->ctype = 0;
	}
	part->temp = bves->temp;
	part->life = bves->tmp2;
	part->tmp = int(bves->pavg[0]);
	part->ctype = int(bves->pavg[1]);
    part->bio = bves->bio;

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

static void transfer_part_to_bves(Particle *part, Particle *bves)
{
	bves->ctype = part->type;
	bves->temp = part->temp;
	bves->tmp2 = part->life;
	bves->pavg[0] = float(part->tmp);
	bves->pavg[1] = float(part->ctype);
    bves->bio = part->bio;
}

static void transfer_bves_to_bves(Particle *src, Particle *dest, bool STOR)
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
    dest->bio = src->bio;
}

static void pushParticle(Simulation * sim, int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q;
	unsigned int notctype = nextColor(sim->parts[i].tmp);
	if (!TYP(sim->parts[i].ctype) || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
		return;
	x = (int)(sim->parts[i].x+0.5f);
	y = (int)(sim->parts[i].y+0.5f);
	if( !(sim->parts[i].tmp&0x200) )
	{
		//normal random push
		rndstore = RNG::Ref().gen();
		// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
		// so can go 5 cycles without regenerating rndstore
		// (although now we use our own randomizer so maybe should reevaluate all the rndstore usages in every element)
		for (q=0; q<3; q++)//try to push 3 times
		{
			rnd = rndstore&7;
			rndstore = rndstore>>3;

			rx = bpos_1_rx[rnd];
			ry = bpos_1_ry[rnd];

			if (BOUNDS_CHECK)
			{
				
				r = sim->pmap[y+ry][x+rx];

				if (!r)
					continue;
				else if ((TYP(r)==PT_BVES || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
				{
					transfer_bves_to_bves(sim->parts+i, sim->parts+(ID(r)), false);
					if (ID(r) > original)
						sim->parts[ID(r)].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
					count++;
					pushParticle(sim, ID(r),count,original);
				}
				else if (TYP(r) == PT_PRTI) //Pass particles into PRTI for a bves speed increase
				{
					int portaltmp = sim->parts[ID(r)].tmp;
					if (portaltmp >= CHANNELS)
						portaltmp = CHANNELS-1;
					else if (portaltmp < 0)
						portaltmp = 0;
					for (int nnx = 0; nnx < 80; nnx++)
						if (!sim->portalp[portaltmp][count][nnx].type)
						{
							Element_BVES_transfer_bves_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]), false);
							count++;
							break;
						}
				}
			}
		}
	}
	else //predefined 1 pixel thick bves movement
	{
		int coords = 7 - ((sim->parts[i].tmp>>10)&7);
		r = sim->pmap[y+ bpos_1_ry[coords]][x+ bpos_1_rx[coords]];
		if ((TYP(r)==PT_BVES || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
		{
			transfer_bves_to_bves(sim->parts+i, sim->parts+(ID(r)), false);
			if (ID(r) > original)
				sim->parts[ID(r)].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
			count++;
			pushParticle(sim, ID(r),count,original);
		}
		else if (TYP(r) == PT_PRTI) //Pass particles into PRTI for a bves speed increase
		{
			int portaltmp = sim->parts[ID(r)].tmp;
			if (portaltmp >= CHANNELS)
				portaltmp = CHANNELS-1;
			else if (portaltmp < 0)
				portaltmp = 0;
			for (int nnx = 0; nnx < 80; nnx++)
				if (!sim->portalp[portaltmp][count][nnx].type)
				{
					Element_BVES_transfer_bves_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]), false);
					count++;
					break;
				}
		}
		else if (!r) //Move particles out of bves automatically, much faster at ends
		{
			rx = bpos_1_rx[coords];
			ry = bpos_1_ry[coords];
			np = sim->create_part(-1,x+rx,y+ry,TYP(sim->parts[i].ctype));
			if (np!=-1)
			{
				Element_BVES_transfer_bves_to_part(sim, sim->parts+i, sim->parts+np, false);
			}
		}

	}
	return;
}