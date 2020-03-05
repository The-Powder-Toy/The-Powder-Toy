#include "simulation/ElementCommon.h"

void Element_PIPE_transfer_pipe_to_part(Simulation * sim, Particle *pipe, Particle *part, bool STOR);
static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
void Element_SOAP_detach(Simulation * sim, int i);

void Element::Element_PRTI()
{
	Identifier = "DEFAULT_PT_PRTI";
	Name = "PRTI";
	Colour = PIXPACK(0xEB5917);
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
	HotAir = -0.005f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 0;

	Weight = 100;

	HeatConduct = 0;
	Description = "Portal IN. Particles go in here. Also has temperature dependent channels. (same as WIFI)";

	Properties = TYPE_SOLID;

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
}

/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/

static int update(UPDATE_FUNC_ARGS)
{
	int fe = 0;

	parts[i].tmp = (int)((parts[i].temp-73.15f)/100+1);
	if (parts[i].tmp >= CHANNELS)
		parts[i].tmp = CHANNELS-1;
	else if (parts[i].tmp < 0)
		parts[i].tmp = 0;

	for (int count = 0; count < 8; count++)
	{
		int rx = sim->portal_rx[count];
		int ry = sim->portal_ry[count];
		if (BOUNDS_CHECK && (rx || ry))
		{
			int r = pmap[y+ry][x+rx];
			if (!r || TYP(r) == PT_STOR)
				fe = 1;
			if (!r || (!(sim->elements[TYP(r)].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)) && TYP(r)!=PT_SPRK && TYP(r)!=PT_STOR))
			{
				r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
			}

			if (TYP(r)==PT_STKM || TYP(r)==PT_STKM2 || TYP(r)==PT_FIGH)
				continue;// Handling these is a bit more complicated, and is done in STKM_interact()

			if (TYP(r) == PT_SOAP)
				Element_SOAP_detach(sim, ID(r));

			for (int nnx=0; nnx<80; nnx++)
				if (!sim->portalp[parts[i].tmp][count][nnx].type)
				{
					if (TYP(r) == PT_STOR)
					{
						if (sim->IsValidElement(parts[ID(r)].tmp) && (sim->elements[parts[ID(r)].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
						{
							// STOR uses same format as PIPE, so we can use this function to do the transfer
							Element_PIPE_transfer_pipe_to_part(sim, parts+(ID(r)), &sim->portalp[parts[i].tmp][count][nnx], true);
							break;
						}
					}
					else
					{
						sim->portalp[parts[i].tmp][count][nnx] = parts[ID(r)];
						if (TYP(r) == PT_SPRK)
							sim->part_change_type(ID(r),x+rx,y+ry,parts[ID(r)].ctype);
						else
							sim->kill_part(ID(r));
						fe = 1;
						break;
					}
				}
		}
	}


	if (fe) {
		int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!sim->parts[i].life) parts[i].life = RNG::Ref().gen();
		if (!sim->parts[i].ctype) parts[i].ctype = RNG::Ref().gen();
		sim->orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (int r = 0; r < 4; r++) {
			if (orbd[r]>1) {
				orbd[r] -= 12;
				if (orbd[r]<1) {
					orbd[r] = RNG::Ref().between(128, 255);
					orbl[r] = RNG::Ref().between(0, 254);
				} else {
					orbl[r] += 2;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = RNG::Ref().between(128, 255);
				orbl[r] = RNG::Ref().between(0, 254);
			}
		}
		sim->orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	} else {
		parts[i].life = 0;
		parts[i].ctype = 0;
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	*firea = 8;
	*firer = 255;
	*fireg = 0;
	*fireb = 0;
	*pixel_mode |= EFFECT_DBGLINES;
	*pixel_mode |= EFFECT_GRAVIN;
	*pixel_mode &= ~PMODE;
	*pixel_mode |= PMODE_ADD;
	return 1;
}
