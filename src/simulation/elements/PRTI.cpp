#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PRTI PT_PRTI 109
Element_PRTI::Element_PRTI()
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

	Temperature = R_TEMP+0.0f	+273.15f;
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

	Update = &Element_PRTI::update;
	Graphics = &Element_PRTI::graphics;
}

/*these are the count values of where the particle gets stored, depending on where it came from
   0 1 2
   7 . 3
   6 5 4
   PRTO does (count+4)%8, so that it will come out at the opposite place to where it came in
   PRTO does +/-1 to the count, so it doesn't jam as easily
*/

//#TPT-Directive ElementHeader Element_PRTI static int update(UPDATE_FUNC_ARGS)
int Element_PRTI::update(UPDATE_FUNC_ARGS)
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
			if (!r || (r&0xFF) == PT_STOR)
				fe = 1;
			if (!r || (!(sim->elements[r&0xFF].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)) && (r&0xFF)!=PT_SPRK && (r&0xFF)!=PT_STOR))
			{
				r = sim->photons[y+ry][x+rx];
				if (!r)
					continue;
			}

			if ((r&0xFF)==PT_STKM || (r&0xFF)==PT_STKM2 || (r&0xFF)==PT_FIGH)
				continue;// Handling these is a bit more complicated, and is done in STKM_interact()

			if ((r&0xFF) == PT_SOAP)
				Element_SOAP::detach(sim, r>>8);

			for (int nnx=0; nnx<80; nnx++)
				if (!sim->portalp[parts[i].tmp][count][nnx].type)
				{
					if ((r&0xFF) == PT_STOR)
					{
						if (sim->IsValidElement(parts[r>>8].tmp) && (sim->elements[parts[r>>8].tmp].Properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
						{
							// STOR uses same format as PIPE, so we can use this function to do the transfer
							Element_PIPE::transfer_pipe_to_part(sim, parts+(r>>8), &sim->portalp[parts[i].tmp][count][nnx]);
							break;
						}
					}
					else
					{
						sim->portalp[parts[i].tmp][count][nnx] = parts[r>>8];
						if ((r&0xFF) == PT_SPRK)
							sim->part_change_type(r>>8,x+rx,y+ry,parts[r>>8].ctype);
						else
							sim->kill_part(r>>8);
						fe = 1;
						break;
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
		for (int r = 0; r < 4; r++) {
			if (orbd[r]>1) {
				orbd[r] -= 12;
				if (orbd[r]<1) {
					orbd[r] = (rand()%128)+128;
					orbl[r] = rand()%255;
				} else {
					orbl[r] += 2;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = (rand()%128)+128;
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


//#TPT-Directive ElementHeader Element_PRTI static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PRTI::graphics(GRAPHICS_FUNC_ARGS)

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


Element_PRTI::~Element_PRTI() {}
