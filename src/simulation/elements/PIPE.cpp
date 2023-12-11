#include "simulation/ElementCommon.h"
#include "PIPE.h"
#include "SOAP.h"

static void props_pipe_to_part(const Particle *pipe, Particle *part, bool STOR);
static void transfer_part_to_pipe(Particle *part, Particle *pipe);
static void transfer_pipe_to_pipe(Particle *src, Particle *dest, bool STOR);
static void pushParticle(Simulation * sim, int i, int count, int original);

void Element::Element_PIPE()
{
	Identifier = "DEFAULT_PT_PIPE";
	Name = "PIPE";
	Colour = 0x444444_rgb;
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

	DefaultProperties.temp = 273.15f;
	HeatConduct = 0;
	Description = "PIPE, moves particles around. Once the BRCK generates, erase some for the exit. Then the PIPE generates and is usable.";

	Properties = TYPE_SOLID|PROP_LIFE_DEC;
	CarriesTypeIn = 1U << FIELD_CTYPE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 10.0f;
	HighPressureTransition = PT_BRMT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	DefaultProperties.life = 60;

	Update = &Element_PIPE_update;
	Graphics = &Element_PIPE_graphics;
}

// 0x000000FF element
// 0x00000100 is single pixel pipe
// 0x00000200 will transfer like a single pixel pipe when in forward mode
// 0x00001C00 forward single pixel pipe direction
// 0x00002000 will transfer like a single pixel pipe when in reverse mode
// 0x0001C000 reverse single pixel pipe direction
// 0x000E0000 PIPE color data stored here

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

const std::array<Vec2<int>, 8> Element_PIPE_offsets = {{
	{ -1, -1 },
	{ -1,  0 },
	{ -1,  1 },
	{  0, -1 },
	{  0,  1 },
	{  1, -1 },
	{  1,  0 },
	{  1,  1 },
}};

void Element_PIPE_transformPatchOffsets(Particle &part, const std::array<int, 8> &offsetMap)
{
	if (part.tmp & 0x00000200) part.tmp = (part.tmp & 0xFFFFE3FF) | (offsetMap[(part.tmp & 0x00001C00) >> 10] << 10);
	if (part.tmp & 0x00002000) part.tmp = (part.tmp & 0xFFFE3FFF) | (offsetMap[(part.tmp & 0x0001C000) >> 14] << 14);
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

int Element_PIPE_update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	if (parts[i].ctype && !elements[TYP(parts[i].ctype)].Enabled)
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
			for (auto rx = -2; rx <= 2; rx++)
			{
				for (auto ry = -2; ry <= 2; ry++)
				{
					if (rx || ry)
					{
						auto r = pmap[y+ry][x+rx];
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
			for (auto rx = -1; rx <= 1; rx++)
			{
				for (auto ry = -1; ry <= 1; ry++)
				{
					if (rx || ry)
					{
						count++;
						auto r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (TYP(r) != PT_PIPE && TYP(r) != PT_PPIP)
							continue;
						unsigned int next = nextColor(parts[i].tmp);
						unsigned int prev = prevColor(parts[i].tmp);
						if (parts[ID(r)].tmp&PFLAG_INITIALIZING)
						{
							parts[ID(r)].tmp |= next;
							parts[ID(r)].tmp &= ~PFLAG_INITIALIZING;
							parts[ID(r)].life = 6;
							// Is a single pixel pipe
							if (parts[i].tmp&0x100)
							{
								// Will transfer to a single pixel pipe
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
				}
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
				auto rndstore = sim->rng.gen();
				auto rnd = rndstore&7;
				auto rx = Element_PIPE_offsets[rnd].X;
				auto ry = Element_PIPE_offsets[rnd].Y;
				auto r = pmap[y+ry][x+rx];
				if(!r)
					r = sim->photons[y+ry][x+rx];
				if (surround_space && !r && TYP(parts[i].ctype))  //creating at end
				{
					auto np = sim->create_part(-1, x+rx, y+ry, TYP(parts[i].ctype));
					if (np!=-1)
					{
						Element_PIPE_transfer_pipe_to_part(sim, parts+i, parts+np, false);
					}
				}
				//try eating particle at entrance
				else if (!TYP(parts[i].ctype) && (elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
				{
					if (TYP(r)==PT_SOAP)
						Element_SOAP_detach(sim, ID(r));
					transfer_part_to_pipe(parts+(ID(r)), parts+i);
					sim->kill_part(ID(r));
				}
				else if (!TYP(parts[i].ctype) && TYP(r)==PT_STOR && sd.IsElement(parts[ID(r)].tmp) && (elements[parts[ID(r)].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
				{
					// STOR stores properties in the same places as PIPE does
					transfer_pipe_to_pipe(parts+(ID(r)), parts+i, true);
				}
			}
		}
	}
	else if (!(parts[i].tmp&(PFLAG_COLORS|PFLAG_INITIALIZING)) && parts[i].life<=10)
	{
		// make a border
		for (auto rx = -2; rx <= 2; rx++)
		{
			for (auto ry = -2; ry <= 2; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					if (!r)
					{
						// BRCK border
						int index = sim->create_part(-1,x+rx,y+ry,PT_BRCK);
						if (parts[i].type == PT_PPIP && index != -1)
							parts[index].tmp = 1;
					}
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
			for (auto rx = -1; rx <= 1; rx++)
			{
				for (auto ry = -1; ry <= 1; ry++)
				{
					if (rx || ry)
					{
						if (!pmap[y+ry][x+rx] && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_ALLOWAIR && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALL && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALLELEC && (sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_EWALL || sim->emap[(y+ry)/CELL][(x+rx)/CELL]))
							parts[i].life=50;
					}
				}
			}
		}
		else if (parts[i].life==5)//check for beginning of pipe single pixel
		{
			int issingle = 1;
			for (auto rx = -1; rx <= 1; rx++)
			{
				for (auto ry = -1; ry <= 1; ry++)
				{
					if (rx || ry)
					{
						auto r = pmap[y+ry][x+rx];
						if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && parts[i].life)
							issingle = 0;
					}
				}
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

int Element_PIPE_graphics(GRAPHICS_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	auto &graphicscache = sd.graphicscache;
	int t = TYP(cpart->ctype);
	if (t>0 && t<PT_NUM && elements[t].Enabled)
	{
		if (t == PT_STKM || t == PT_STKM2 || t == PT_FIGH)
			return 0;
		if (graphicscache[t].isready)
		{
			*pixel_mode = graphicscache[t].pixel_mode;
			*cola = graphicscache[t].cola;
			*colr = graphicscache[t].colr;
			*colg = graphicscache[t].colg;
			*colb = graphicscache[t].colb;
			*firea = graphicscache[t].firea;
			*firer = graphicscache[t].firer;
			*fireg = graphicscache[t].fireg;
			*fireb = graphicscache[t].fireb;
		}
		else
		{
			// We emulate the graphics of the stored particle. We need a const Particle *cpart to pass to the graphics function,
			// but we don't have a Particle that is populated the way the graphics function expects, so we construct a temporary
			// one and present that to it.
			//
			// Native graphics functions are well-behaved and use the cpart we give them, no questions asked, so we can just have
			// the Particle on stack. Swapping the pointers in cpart with tpart takes care of passing the particle on stack to the
			// native graphics function. Lua graphics functions are more complicated to appease: they access particle data through the
			// particle ID, so not only do we have to give them a correctly populated Particle, it also has to be somewhere in Simulation.
			// luaGraphicsWrapper takes care of this.
			RGB<uint8_t> colour = elements[t].Colour;
			*colr = colour.Red;
			*colg = colour.Green;
			*colb = colour.Blue;
			auto *graphics = elements[t].Graphics;
			if (graphics)
			{
				Particle tpart;
				props_pipe_to_part(cpart, &tpart, false);
				auto *prevPipeSubcallCpart = gfctx.pipeSubcallCpart;
				auto *prevPipeSubcallTpart = gfctx.pipeSubcallTpart;
				gfctx.pipeSubcallCpart = cpart;
				gfctx.pipeSubcallTpart = &tpart;
				cpart = gfctx.pipeSubcallTpart;
				graphics(GRAPHICS_FUNC_SUBCALL_ARGS);
				cpart = gfctx.pipeSubcallCpart;
				gfctx.pipeSubcallCpart = prevPipeSubcallCpart;
				gfctx.pipeSubcallTpart = prevPipeSubcallTpart;
			}
			else
			{
				Element::defaultGraphics(GRAPHICS_FUNC_SUBCALL_ARGS);
			}
		}
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

static void props_pipe_to_part(const Particle *pipe, Particle *part, bool STOR)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
	// STOR also calls this function to move particles from STOR to PRTI
	// PIPE was changed, so now PIPE and STOR don't use the same particle storage format
	if (STOR)
	{
		part->type = TYP(pipe->tmp);
	}
	else
	{
		part->type = TYP(pipe->ctype);
	}
	part->temp = pipe->temp;
	part->life = pipe->tmp2;
	part->tmp = pipe->tmp3;
	part->ctype = pipe->tmp4;

	if (!(elements[part->type].Properties & TYPE_ENERGY))
	{
		part->vx = 0.0f;
		part->vy = 0.0f;
	}
	part->tmp2 = 0;
	part->flags = 0;
	part->dcolour = 0;
}

void Element_PIPE_transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR)
{
	props_pipe_to_part(pipe, part, STOR);
	if (STOR)
	{
		pipe->tmp = 0;
	}
	else
	{
		pipe->ctype = 0;
	}
}

static void transfer_part_to_pipe(Particle *part, Particle *pipe)
{
	pipe->ctype = part->type;
	pipe->temp = part->temp;
	pipe->tmp2 = part->life;
	pipe->tmp3 = part->tmp;
	pipe->tmp4 = part->ctype;
}

static void transfer_pipe_to_pipe(Particle *src, Particle *dest, bool STOR)
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
	dest->tmp3 = src->tmp3;
	dest->tmp4 = src->tmp4;
}

static void pushParticle(Simulation * sim, int i, int count, int original)
{
	unsigned int notctype = nextColor(sim->parts[i].tmp);
	if (!TYP(sim->parts[i].ctype) || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
		return;
	auto x = (int)(sim->parts[i].x+0.5f);
	auto y = (int)(sim->parts[i].y+0.5f);
	if( !(sim->parts[i].tmp&0x200) )
	{
		//normal random push
		auto rndstore = sim->rng.gen();
		// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
		// so can go 5 cycles without regenerating rndstore
		// (although now we use our own randomizer so maybe should reevaluate all the rndstore usages in every element)
		for (auto q=0; q<3; q++)//try to push 3 times
		{
			auto rnd = rndstore&7;
			rndstore = rndstore>>3;
			auto rx = Element_PIPE_offsets[rnd].X;
			auto ry = Element_PIPE_offsets[rnd].Y;
			auto r = sim->pmap[y+ry][x+rx];
			if (!r)
				continue;
			else if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
			{
				transfer_pipe_to_pipe(sim->parts+i, sim->parts+(ID(r)), false);
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
						Element_PIPE_transfer_pipe_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]), false);
						count++;
						break;
					}
			}
		}
	}
	else //predefined 1 pixel thick pipe movement
	{
		int coords = 7 - ((sim->parts[i].tmp>>10)&7);
		auto r = sim->pmap[y+ Element_PIPE_offsets[coords].Y][x+ Element_PIPE_offsets[coords].X];
		if ((TYP(r)==PT_PIPE || TYP(r) == PT_PPIP) && (sim->parts[ID(r)].tmp&PFLAG_COLORS) != notctype && !TYP(sim->parts[ID(r)].ctype))
		{
			transfer_pipe_to_pipe(sim->parts+i, sim->parts+(ID(r)), false);
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
					Element_PIPE_transfer_pipe_to_part(sim, sim->parts+i, &(sim->portalp[portaltmp][count][nnx]), false);
					count++;
					break;
				}
		}
		else if (!r) //Move particles out of pipe automatically, much faster at ends
		{
			auto rx = Element_PIPE_offsets[coords].X;
			auto ry = Element_PIPE_offsets[coords].Y;
			auto np = sim->create_part(-1,x+rx,y+ry,TYP(sim->parts[i].ctype));
			if (np!=-1)
			{
				Element_PIPE_transfer_pipe_to_part(sim, sim->parts+i, sim->parts+np, false);
			}
		}

	}
	return;
}
