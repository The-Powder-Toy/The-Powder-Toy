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
    Description = "Particle Ray Emitter. Creates a beam of particles set by ctype, range is set by tmp";
    
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
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1, partsRemaining = 255;
	if (parts[i].tmp)
		partsRemaining = parts[i].tmp;
	if(parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled) {
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_CRAY && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
					}
				}
	} else if (parts[i].life==0) {
		unsigned int colored = 0;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
						int nostop = (parts[r>>8].ctype==PT_INST)?1:0;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!r) {
								int nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, parts[i].ctype);
								if (nr!=-1) {
									parts[nr].dcolour = colored;
									if(!--partsRemaining)
										docontinue = 0;
								}
							} else if ((r&0xFF)==PT_FILT) {//get color if passed through FILT
								colored = wavelengthToDecoColour(parts[r>>8].ctype);
							} else if ((r&0xFF)==PT_CRAY || nostop) {
								docontinue = 1;
							} else if(destroy && (r&0xFF != PT_DMND)) {
								sim->kill_part(r>>8);
								if(!--partsRemaining)
									docontinue = 0;
							} else {
								docontinue = 0;
							}
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