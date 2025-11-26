#include "simulation/ElementCommon.h"
#include "PLNT.h"
#include <algorithm>

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_PLNT()
{
	Identifier = "DEFAULT_PT_PLNT";
	Name = "PLNT";
	Colour = 0x0CAC00_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
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

	Flammable = 20;
	Explosive = 0;
	Meltable = 0;
	Hardness = 10;
	PhotonReflectWavelengths = 0x0007C000;

	Weight = 100;

	HeatConduct = 65;
	Description = "Plant, drinks water and grows.";

	Properties = TYPE_SOLID|PROP_NEUTPENETRATE|PROP_LIFE_DEC;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f;
	HighTemperatureTransition = PT_FIRE;

	Update = &update;
	Graphics = &graphics;
}

// Fields used for tree growth (see also PLNT.h):
// ctype: tree or usual PLNT (1 bit), phase (2 bits), direction (3 bits), colour (6 bits), water (8 bits)
// life: used as a counter in growth
// tmp-tmp4: branching bitmask (5 bits), new phases (5x2 bits)

static int update(UPDATE_FUNC_ARGS)
{
	// Select tree growth code or usual PLNT code
	if (parts[i].ctype & 1)
	{
		auto temp = parts[i].temp;

		// Hot and cold temperature kills tree growth
		if (temp > 373.15f || temp < 223.15f)
		{
			parts[i].ctype = 0;
			parts[i].tmp = 0;
			parts[i].tmp2 = 0;
			parts[i].tmp3 = 0;
			parts[i].tmp4 = 0;

			return 0;
		}

		// No growth happens at bad temperatures
		if (temp > 343.15f || temp < 278.15f)
			return 0;

		// Slow things down
		if (sim->rng.chance(9, 10))
			return 0;

		// Current phase, direction and life
		int phase = (parts[i].ctype>>PLNT_PHASE) & 0x3;
		int dir = (parts[i].ctype>>PLNT_DIR) & 0x7;
		int life = (parts[i].ctype>>PLNT_LIFE) & 0xFF;

		int prog = 0;

		// Select a program for the current phase
		switch (phase)
		{
			case 0: prog = parts[i].tmp; break;
			case 1: prog = parts[i].tmp2; break;
			case 2: prog = parts[i].tmp3; break;
			case 3: prog = parts[i].tmp4; break;
		}

		int si = -1; // Used for create_part calls
		bool stopped = false; // Did the cell stop growing?

		int down = detectDown(sim, x, y); // Down gravity direction (8 means no direction)

		// Do we continue growing?
		if (life > 0)
		{
			// Do we grow forward or branch?
			if (!parts[i].life)
			{
				// Branching
				int bmask = prog&0x1F;
				int phases = prog>>5;

				// Can we branch or it's a dead end?
				if (bmask)
				{
					// Loop through all possible branching directions
					for (int phi = 0; phi < 5; phi++)
					{
						if ((bmask>>phi)&0x1)
						{
							int ndir = (dir + phi + 6) % 8; // phi = 0 is -90 deg, phi = 1 is -45 deg etc
							int nphase = phases&0x3;
							int nlife = (2*life)/3;

							// Discourage branches which go down
							if (down < 8)
							{
								// Down
								if(ndir == down)
									nlife = life/3;

								// Down-left and down-right
								if(ndir == ((down+1)%8) || ndir == ((down+7)%8))
									nlife = life/3;

								// Sideways
								if(life > 4 && (ndir == ((down+2)%8) || ndir == ((down+6)%8)))
									nlife = life/2;
							}

							si = sim->create_part(-1, x+dir3x3[ndir].X, y+dir3x3[ndir].Y, PT_PLNT);
							if (si >= 0)
							{
								parts[si].ctype = ((nlife & 0xFF) << PLNT_LIFE) | (parts[i].ctype & (0x3f << PLNT_COLOUR))
										| ((ndir & 7) << PLNT_DIR) | ((nphase & 3) << PLNT_PHASE) | 0x1;

								// Reduce the length of diagonal branches by 1/sqrt(2) ~= 0.7
								parts[si].life = (ndir%2 == 1) ? 7*nlife : 10*nlife;

								parts[si].tmp = parts[i].tmp;
								parts[si].tmp2 = parts[i].tmp2;
								parts[si].tmp3 = parts[i].tmp3;
								parts[si].tmp4 = parts[i].tmp4;
							}
						}

						phases = phases>>2;
					}
				}
				else
					stopped = true;
			}
			else
			{
				// Grow forward
				si = sim->create_part(-1, x+dir3x3[dir].X, y+dir3x3[dir].Y, PT_PLNT);
				if (si >= 0)
				{
					parts[si].ctype = ((life & 0xFF) << PLNT_LIFE) | (parts[i].ctype & (0x3f << PLNT_COLOUR))
							| ((dir & 7) << PLNT_DIR) | ((phase & 3) << PLNT_PHASE) | 0x1;

					parts[si].life = parts[i].life;
					parts[si].tmp = parts[i].tmp;
					parts[si].tmp2 = parts[i].tmp2;
					parts[si].tmp3 = parts[i].tmp3;
					parts[si].tmp4 = parts[i].tmp4;
				}
			}

			if (!stopped)
			{
				// Create a stem
				if (phase || life < 12)
				{
					sim->create_part(i, x, y, PT_WOOD);

					parts[i].ctype = 0;
					parts[i].life = 0;
					parts[i].tmp = 0;
					parts[i].tmp2 = 0;
					parts[i].tmp3 = 0;
					parts[i].tmp4 = 0;
				}
				else
				{
					// Thick stem
					sim->create_part(i, x, y, PT_GOO);

					parts[i].ctype = 0;
					parts[i].life = 0;
					parts[i].tmp = 0;
					parts[i].tmp2 = 0;
					parts[i].tmp3 = 0;
					parts[i].tmp4 = 0;

					int left_dir = (dir+6)%8;
					int right_dir = (dir+2)%8;

					// Correction for diagonal directions
					if (dir%2)
					{
						left_dir = (dir+5)%8;
						right_dir = (dir+3)%8;
					}

					sim->create_part(-1, x+dir3x3[left_dir].X, y+dir3x3[left_dir].Y, PT_WOOD);
					sim->create_part(-1, x+dir3x3[right_dir].X, y+dir3x3[right_dir].Y, PT_WOOD);
				}
			}
		}
		else
			stopped = true;

		// Stop growing
		if (stopped)
		{
			// Shoot out a seed
			if (sim->rng.chance(1, 10))
			{
				si = sim->create_part(-1, x+2*dir3x3[dir].X, y+2*dir3x3[dir].Y, PT_SEED);
				if (si >= 0)
				{
					parts[si].vx = dir3x3[dir].X;
					parts[si].vy = dir3x3[dir].Y;

					// Inherit genome
					parts[si].ctype = parts[i].ctype & (0x3f << PLNT_COLOUR); // Preserve only the colour
					parts[si].tmp = parts[i].tmp;
					parts[si].tmp2 = parts[i].tmp2;
					parts[si].tmp3 = parts[i].tmp3;
					parts[si].tmp4 = parts[i].tmp4;
				}
			}

			// Turn into usual plant (but preserve color)
			// We set direction to 7 to tell graphics() that PLNT needs coloring
			parts[i].ctype = (parts[i].ctype & (0x3f << PLNT_COLOUR)) | (7 << PLNT_DIR);
			parts[i].life = 0;
			parts[i].tmp = 0;
			parts[i].tmp2 = 0;
			parts[i].tmp3 = 0;
			parts[i].tmp4 = 0;
		}

		return 1;
	}
	else
	{ // Usual PLNT code
		for (auto rx = -1; rx <= 1; rx++)
		{
			for (auto ry = -1; ry <= 1; ry++)
			{
				if (rx || ry)
				{
					auto r = pmap[y+ry][x+rx];
					switch (TYP(r))
					{
						case PT_WATR:
							if (sim->rng.chance(1, 50))
							{
								auto np = sim->create_part(ID(r),x+rx,y+ry,PT_PLNT);
								if (np<0) continue;
								parts[np].life = 0;
							}
							break;
						case PT_LAVA:
							if (sim->rng.chance(1, 50))
							{
								sim->part_change_type(i,x,y,PT_FIRE);
								parts[i].life = 4;
							}
							break;
						case PT_SMKE:
						case PT_CO2:
							if (sim->rng.chance(1, 50))
							{
								sim->kill_part(ID(r));
								parts[i].life = sim->rng.between(60, 119);
							}
							break;
						case PT_WOOD:
							{
								auto rndstore = sim->rng.gen();
								if (surround_space && !(rndstore%4) && parts[i].tmp==1)
								{
									rndstore >>= 3;
									int nnx = (rndstore%3) -1;
									rndstore >>= 2;
									int nny = (rndstore%3) -1;
									if (nnx || nny)
									{
										if (pmap[y+ry+nny][x+rx+nnx])
											continue;
										auto np = sim->create_part(-1,x+rx+nnx,y+ry+nny,PT_VINE);
										if (np<0) continue;
										parts[np].temp = parts[i].temp;
									}
								}
							}
							break;
						default:
							continue;
					}
				}
			}
		}
		if (parts[i].life==2)
		{
			for (auto rx = -1; rx <= 1; rx++)
			{
				for (auto ry = -1; ry <= 1; ry++)
				{
					if (rx || ry)
					{
						auto r = pmap[y+ry][x+rx];
						if (!r)
							sim->create_part(-1,x+rx,y+ry,PT_O2);
					}
				}
			}
			parts[i].life = 0;
		}
		if (parts[i].temp > 350 && parts[i].temp > parts[i].tmp2)
			parts[i].tmp2 = (int)parts[i].temp;
	}
	return 0;
}

constexpr std::array<std::array<int, 3>, 8> leafColor = {{
    {{ 243, 246, 244 }}, // White
    {{ 255, 223, 50  }}, // Yellow
    {{ 255, 183, 197 }}, // Pink
    {{ 250, 0,   25  }}, // Red
    {{ 128, 206, 196 }}, // Cyan / light blue
    {{ 127, 255, 0   }}, // Bright green
    {{ 0,   74,  178 }}, // Blue
    {{ 12,  172, 0   }}  // Usual PLNT green
}};

static int graphics(GRAPHICS_FUNC_ARGS)
{
	if (cpart->ctype == 0)
	{
		float maxtemp = std::max((float)cpart->tmp2, cpart->temp);
		if (maxtemp > 300)
		{
			*colr += (int)restrict_flt((maxtemp-300)/5,0,58);
			*colg -= (int)restrict_flt((maxtemp-300)/2,0,102);
			*colb += (int)restrict_flt((maxtemp-300)/5,0,70);
		}
		if (maxtemp < 273)
		{
			*colg += (int)restrict_flt((273-maxtemp)/4,0,255);
			*colb += (int)restrict_flt((273-maxtemp)/1.5,0,255);
		}
	}
	else
	{
		// Set tree-grown PLNT color
		int colour = (cpart->ctype >> PLNT_COLOUR) & 0x3f;

		int cyan = (colour & 0b110000) != 0;
		int magenta = (colour & 0b001100) != 0;
		int yellow = (colour & 0b000011) != 0;

		int ind = 4*cyan + 2*magenta + yellow;

		if (0 <= ind && ind < 8)
		{
			*colr = leafColor[ind][0];
			*colg = leafColor[ind][1];
			*colb = leafColor[ind][2];
		}
	}

	return 0;
}

// Inverse of dir3x3 table
constexpr std::array<std::array<int, 3>, 3> invDir = {{
    {{ 1, 2, 3 }},
    {{ 0, 8, 4 }},
    {{ 7, 6, 5 }},
}};

int detectDown(Simulation *sim, int x, int y)
{
	float pGravX = 0;
	float pGravY = 0;

	sim->GetGravityField(x, y, 1.0f, 1.0f, pGravX, pGravY);

	int gravx = 1;
	int gravy = 1;

	if (pGravX > 0.5f)
		gravx = 2;
	if (pGravX < -0.5f)
		gravx = 0;
	if (pGravY > 0.5f)
		gravy = 2;
	if (pGravY < -0.5f)
		gravy = 0;

	return invDir[gravx][gravy];
}
