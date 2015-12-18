#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PRTO PT_PRTO 110
Element_PRTO::Element_PRTO()
{
	Identifier = "DEFAULT_PT_PRTO";
	Name = "PRTO";
	Colour = PIXPACK(0x0020EB);
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
	HotAir = 0.005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Portal OUT. Particles come out here. Also has temperature dependent channels. (same as WIFI)";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_PRTO::update;
	Graphics = &Element_PRTO::graphics;
}

/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/

//#TPT-Directive ElementHeader Element_PRTO static int update(UPDATE_FUNC_ARGS)
int Element_PRTO::update(UPDATE_FUNC_ARGS)
{
	int r, nnx, rx, ry, np, fe = 0;
	int count = 0;
	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp>=CHANNELS) parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp<0) parts[i].tmp = 0;
	for (count=0; count<8; count++)
	{
		rx = sim->portal_rx[count];
		ry = sim->portal_ry[count];
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
				{
					fe = 1;
					for ( nnx =0 ; nnx<80; nnx++)
					{
						int randomness = (count + rand()%3-1 + 4)%8;//add -1,0,or 1 to count
						if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_SPRK)// TODO: make it look better, spark creation
						{
							sim->create_part(-1,x+1,y,PT_SPRK);
							sim->create_part(-1,x+1,y+1,PT_SPRK);
							sim->create_part(-1,x+1,y-1,PT_SPRK);
							sim->create_part(-1,x,y-1,PT_SPRK);
							sim->create_part(-1,x,y+1,PT_SPRK);
							sim->create_part(-1,x-1,y+1,PT_SPRK);
							sim->create_part(-1,x-1,y,PT_SPRK);
							sim->create_part(-1,x-1,y-1,PT_SPRK);
							memset(&sim->portalp[parts[i].tmp][randomness][nnx], 0, sizeof(Particle));
							break;
						}
						else if (sim->portalp[parts[i].tmp][randomness][nnx].type)
						{
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
								sim->player.spwn = 0;
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
								sim->player2.spwn = 0;
							if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
							{
								sim->fighcount--;
								sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 0;
							}
							np = sim->create_part(-1, x+rx, y+ry, sim->portalp[parts[i].tmp][randomness][nnx].type);
							if (np<0)
							{
								if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM)
									sim->player.spwn = 1;
								if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_STKM2)
									sim->player2.spwn = 1;
								if (sim->portalp[parts[i].tmp][randomness][nnx].type==PT_FIGH)
								{
									sim->fighcount++;
									sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
								}
								continue;
							}
							if (parts[np].type==PT_FIGH)
							{
								// Release the fighters[] element allocated by create_part, the one reserved when the fighter went into the portal will be used
								sim->fighters[(unsigned char)parts[np].tmp].spwn = 0;
								sim->fighters[(unsigned char)sim->portalp[parts[i].tmp][randomness][nnx].tmp].spwn = 1;
							}
							if (sim->portalp[parts[i].tmp][randomness][nnx].vx == 0.0f && sim->portalp[parts[i].tmp][randomness][nnx].vy == 0.0f)
							{
								// particles that have passed from PIPE into PRTI have lost their velocity, so use the velocity of the newly created particle if the particle in the portal has no velocity
								float tmp_vx = parts[np].vx;
								float tmp_vy = parts[np].vy;
								parts[np] = sim->portalp[parts[i].tmp][randomness][nnx];
								parts[np].vx = tmp_vx;
								parts[np].vy = tmp_vy;
							}
							else
								parts[np] = sim->portalp[parts[i].tmp][randomness][nnx];
							parts[np].x = x+rx;
							parts[np].y = y+ry;
							memset(&sim->portalp[parts[i].tmp][randomness][nnx], 0, sizeof(Particle));
							break;
						}
					}
				}
			}
	}
	if (fe) {
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!sim->parts[i].life) parts[i].life = rand()*rand()*rand();
		if (!sim->parts[i].ctype) parts[i].ctype = rand()*rand()*rand();
		sim->orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (r = 0; r < 4; r++) {
			if (orbd[r]<254) {
				orbd[r] += 16;
				if (orbd[r]>254) {
					orbd[r] = 0;
					orbl[r] = rand()%255;
				}
				else
				{
					orbl[r] += 1;
					orbl[r] = orbl[r]%255;
				}
				//orbl[r] += 1;
				//orbl[r] = orbl[r]%255;
			} else {
				orbd[r] = 0;
				orbl[r] = rand()%255;
			}
		}
		sim->orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	} else {
		parts[i].life = 0;
		parts[i].ctype = 0;
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_PRTO static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PRTO::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 8;
	*firer = 0;
	*fireg = 0;
	*fireb = 255;
	*pixel_mode |= EFFECT_DBGLINES;
	*pixel_mode |= EFFECT_GRAVOUT;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}


Element_PRTO::~Element_PRTO() {}
