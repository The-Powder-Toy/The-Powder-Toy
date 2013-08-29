#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_ARAY PT_ARAY 126
Element_ARAY::Element_ARAY()
{
	Identifier = "DEFAULT_PT_ARAY";
	Name = "ARAY";
	Colour = PIXPACK(0xFFBB00);
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
	Description = "Ray Emitter. Rays create points when they collide.";
	
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
	
	Update = &Element_ARAY::update;
	
}

//#TPT-Directive ElementHeader Element_ARAY static int update(UPDATE_FUNC_ARGS)
int Element_ARAY::update(UPDATE_FUNC_ARGS)
 {
	int r, nxx, nyy, docontinue, nxi, nyi, rx, ry, nr, ry1, rx1;
	if (parts[i].life==0) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_SPRK && parts[r>>8].life==3) {
						int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
						int nostop = (parts[r>>8].ctype==PT_INST)?1:0;
						int colored = 0;
						for (docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi) {
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)) {
								break;
							}
							r = pmap[y+nyi+nyy][x+nxi+nxx];
							if (!r) {
								int nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
								if (nr!=-1) {
									if (destroy) {//if it came from PSCN
										parts[nr].tmp = 2;
										parts[nr].life = 2;
									} else
										parts[nr].ctype = colored;
									parts[nr].temp = parts[i].temp;
								}
							} else if (!destroy) {
								if ((r&0xFF)==PT_BRAY) {
									//cases for hitting different BRAY modes
									switch(parts[r>>8].tmp) {
									case 0://normal white
										if (nyy!=0 || nxx!=0) {
											parts[r>>8].life = 1020;//makes it last a while
											parts[r>>8].tmp = 1;
											if (!parts[r>>8].ctype)//and colors it if it isn't already
												parts[r>>8].ctype = colored;
										}
									case 2://red bray, stop
									default://stop any other random tmp mode
										docontinue = 0;//then stop it
										break;
									case 1://long life, reset it
										parts[r>>8].life = 1020;
										//docontinue = 1;
										break;
									}
								} else if ((r&0xFF)==PT_FILT) {//get color if passed through FILT
									colored = Element_FILT::interactWavelengths(&parts[r>>8], colored);
									//this if prevents BRAY from stopping on certain materials
								} else if ((r&0xFF)!=PT_STOR && (r&0xFF)!=PT_INWR && ((r&0xFF)!=PT_SPRK || parts[r>>8].ctype!=PT_INWR) && (r&0xFF)!=PT_ARAY && (r&0xFF)!=PT_WIFI && !((r&0xFF)==PT_SWCH && parts[r>>8].life>=10)) {
									if (nyy!=0 || nxx!=0) {
										sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_SPRK);
									}
									if (!(nostop && parts[r>>8].type==PT_SPRK && parts[r>>8].ctype >= 0 && parts[r>>8].ctype < PT_NUM && (sim->elements[parts[r>>8].ctype].Properties&PROP_CONDUCTS))) {
										docontinue = 0;
									} else {
										docontinue = 1;
									}
								} else if((r&0xFF)==PT_STOR) {
									if(parts[r>>8].tmp)
									{
										//Cause STOR to release
										for(ry1 = 1; ry1 >= -1; ry1--){
											for(rx1 = 0; rx1 >= -1 && rx1 <= 1; rx1 = -rx1-rx1+1){
												int np = sim->create_part(-1, x+nxi+nxx+rx1, y+nyi+nyy+ry1, parts[r>>8].tmp);
												if (np!=-1)
												{
													parts[np].temp = parts[r>>8].temp;
													parts[np].life = parts[r>>8].tmp2;
													parts[np].tmp = parts[r>>8].pavg[0];
													parts[np].ctype = parts[r>>8].pavg[1];
													parts[r>>8].tmp = 0;
													parts[r>>8].life = 10;
													break;
												}
											}
										}
									}
									else
									{
										parts[r>>8].life = 10;
									}
								}
							} else if (destroy) {
								if ((r&0xFF)==PT_BRAY) {
									parts[r>>8].life = 1;
									docontinue = 1;
									//this if prevents red BRAY from stopping on certain materials
								} else if ((r&0xFF)==PT_STOR || (r&0xFF)==PT_INWR || ((r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_INWR) || (r&0xFF)==PT_ARAY || (r&0xFF)==PT_WIFI || (r&0xFF)==PT_FILT || ((r&0xFF)==PT_SWCH && parts[r>>8].life>=10)) {
									if((r&0xFF)==PT_STOR)
									{
										parts[r>>8].tmp = 0;
										parts[r>>8].life = 0;
									}
									docontinue = 1;
								} else {
									docontinue = 0;
								}
							}
						}
					}
					//parts[i].life = 4;
				}
	}
	return 0;
}


Element_ARAY::~Element_ARAY() {}
