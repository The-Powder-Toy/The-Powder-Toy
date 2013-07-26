#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_CRAY PT_CRAY 167
Element_CRAY::Element_CRAY()
{
	Identifier = "DEFAULT_PT_CRAY";
	Name = "CRAY";
	Colour = PIXPACK(0xBBFF00);
	MenuVisible = 1;
	MenuSection = SC_ELEC;
	Enabled = 1;
	
	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;
	
	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 1;
	
	Weight = 100;
	
	Temperature = R_TEMP+0.0f +273.15f;
	HeatConduct = 0;
	Description = "Particle Ray Emitter. Creates a beam of particles set by its ctype, with a range set by tmp.";
	
	State = ST_SOLID;
	Properties = TYPE_SOLID|PROP_LIFE_DEC;
	
	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;
	
	Update = &Element_CRAY::update;
	
}

//#TPT-Directive ElementHeader Element_CRAY static int update(UPDATE_FUNC_ARGS)
int Element_CRAY::update(UPDATE_FUNC_ARGS)
 {
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1;
	// set ctype to things that touch it if it doesn't have one already
	if(parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled) {
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_CRAY && (r&0xFF)!=PT_PSCN && (r&0xFF)!=PT_INST && (r&0xFF)!=PT_METL && (r&0xFF)!=PT_SPRK && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						parts[i].temp = parts[r>>8].temp;
					}
				}
	} else if (parts[i].life==0) { // only fire when life is 0, but nothing sets the life right now
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) { //spark found, start creating
						unsigned int colored = 0;
						bool destroy = parts[r>>8].ctype==PT_PSCN;
						bool nostop = parts[r>>8].ctype==PT_INST;
						bool createSpark = (parts[r>>8].ctype==PT_INWR);
						int partsRemaining = 255;
						if (parts[i].tmp) //how far it shoots
							partsRemaining = parts[i].tmp;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!sim->IsWallBlocking(x+nxi+nxx, y+nyi+nyy, parts[i].ctype) && (!sim->pmap[y+nyi+nyy][x+nxi+nxx] || createSpark)) { // create, also set color if it has passed through FILT
								int nr;
								if (parts[i].ctype == PT_LIFE)
									nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype|(parts[i].tmp2<<8));
								else
									nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype);
								if (nr!=-1) {
									parts[nr].dcolour = colored;
									parts[nr].temp = parts[i].temp;
									if(!--partsRemaining)
										docontinue = 0;
								}
							} else if ((r&0xFF)==PT_FILT) { // get color if passed through FILT
								colored = wavelengthToDecoColour(parts[r>>8].ctype);
							} else if ((r&0xFF) == PT_CRAY || nostop) {
								docontinue = 1;
							} else if(destroy && r && ((r&0xFF) != PT_DMND)) {
								sim->kill_part(r>>8);
								if(!--partsRemaining)
									docontinue = 0;
							}
							else
								docontinue = 0;
							if(!partsRemaining)
								docontinue = 0;
						}
					}
				}
	}
	return 0;
}
//#TPT-Directive ElementHeader Element_CRAY static unsigned int wavelengthToDecoColour(int wavelength)
unsigned int Element_CRAY::wavelengthToDecoColour(int wavelength)
{
	int colr = 0, colg = 0, colb = 0, x;
	unsigned int dcolour = 0;
	for (x=0; x<12; x++) {
		colr += (wavelength >> (x+18)) & 1;
		colb += (wavelength >>  x)     & 1;
	}
	for (x=0; x<12; x++)
		colg += (wavelength >> (x+9))  & 1;
	x = 624/(colr+colg+colb+1);
	colr *= x;
	colg *= x;
	colb *= x;

	if(colr > 255) colr = 255;
	else if(colr < 0) colr = 0;
	if(colg > 255) colg = 255;
	else if(colg < 0) colg = 0;
	if(colb > 255) colb = 255;
	else if(colb < 0) colb = 0;

	return (255<<24) | (colr<<16) | (colg<<8) | colb;
}


Element_CRAY::~Element_CRAY() {}
