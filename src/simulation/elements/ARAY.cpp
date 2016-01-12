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
	if (!parts[i].life)
	{
		for (int rx = -1; rx <= 1; rx++)
			for (int ry = -1; ry <= 1; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					int r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF) == PT_SPRK && parts[r>>8].life == 3)
					{
						bool isBlackDeco = false;
						int destroy = (parts[r>>8].ctype==PT_PSCN) ? 1 : 0;
						int nostop = (parts[r>>8].ctype==PT_INST) ? 1 : 0;
						int colored = 0, rt;
						for (int docontinue = 1, nxx = 0, nyy = 0, nxi = rx*-1, nyi = ry*-1; docontinue; nyy+=nyi, nxx+=nxi)
						{
							if (!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0))
								break;

							r = pmap[y+nyi+nyy][x+nxi+nxx];
							rt = r & 0xFF;
							r = r >> 8;
							if (!rt)
							{
								int nr = sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
								if (nr != -1)
								{
									// if it came from PSCN
									if (destroy)
									{
										parts[nr].tmp = 2;
										parts[nr].life = 2;
									}
									else
										parts[nr].ctype = colored;
									parts[nr].temp = parts[i].temp;
									if (isBlackDeco)
										parts[nr].dcolour = 0xFF000000;
								}
							}
							else if (!destroy)
							{
								if (rt == PT_BRAY)
								{
									// cases for hitting different BRAY modes
									switch(parts[r].tmp)
									{
									// normal white
									case 0:
										if (nyy != 0 || nxx !=0)
										{
											parts[r].life = 1020; // makes it last a while
											parts[r].tmp = 1;
											if (!parts[r].ctype) // and colors it if it isn't already
												parts[r].ctype = colored;
										}
									// red bray or any other random tmp mode, stop
									case 2:
									default:
										docontinue = 0;
										break;
									// long life, reset it
									case 1:
										parts[r].life = 1020;
										//docontinue = 1;
										break;
									}
									if (isBlackDeco)
										parts[r].dcolour = 0xFF000000;
								}
								// get color if passed through FILT
								else if (rt == PT_FILT)
								{
									if (parts[r].tmp != 6)
									{
										colored = Element_FILT::interactWavelengths(&parts[r], colored);
										if (!colored)
											break;
									}
									isBlackDeco = (parts[r].dcolour==0xFF000000);
									parts[r].life = 4;
								}
								else if (rt == PT_STOR)
								{
									if (parts[r].tmp)
									{
										//Cause STOR to release
										for (int ry1 = 1; ry1 >= -1; ry1--)
										{
											for (int rx1 = 0; rx1 >= -1 && rx1 <= 1; rx1 = -rx1 - rx1 + 1)
											{
												int np = sim->create_part(-1, x + nxi + nxx + rx1, y + nyi + nyy + ry1, parts[r].tmp&0xFF);
												if (np != -1)
												{
													parts[np].temp = parts[r].temp;
													parts[np].life = parts[r].tmp2;
													parts[np].tmp = parts[r].pavg[0];
													parts[np].ctype = parts[r].pavg[1];
													parts[r].tmp = 0;
													parts[r].life = 10;
													break;
												}
											}
										}
									}
									else
									{
										parts[r].life = 10;
									}
								// this if prevents BRAY from stopping on certain materials
								}
								else if (rt != PT_INWR && (rt != PT_SPRK || parts[r].ctype != PT_INWR) && rt != PT_ARAY && rt != PT_WIFI && !(rt == PT_SWCH && parts[r].life >= 10))
								{
									if (nyy!=0 || nxx!=0)
										sim->create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_SPRK);

									if (!(nostop && parts[r].type==PT_SPRK && parts[r].ctype >= 0 && parts[r].ctype < PT_NUM && (sim->elements[parts[r].ctype].Properties&PROP_CONDUCTS)))
										docontinue = 0;
									else
										docontinue = 1;
								}
							}
							else if (destroy)
							{
								if (rt == PT_BRAY)
								{
									parts[r].tmp = 2;
									parts[r].life = 1;
									docontinue = 1;
									if (isBlackDeco)
										parts[r].dcolour = 0xFF000000;
								//this if prevents red BRAY from stopping on certain materials
								}
								else if (rt==PT_STOR || rt==PT_INWR || (rt==PT_SPRK && parts[r].ctype==PT_INWR) || rt==PT_ARAY || rt==PT_WIFI || rt==PT_FILT || (rt==PT_SWCH && parts[r].life>=10))
								{
									if (rt == PT_STOR)
									{
										parts[r].tmp = 0;
										parts[r].life = 0;
									}
									else if (rt == PT_FILT)
									{
										isBlackDeco = (parts[r].dcolour==0xFF000000);
										parts[r].life = 2;
									}
									docontinue = 1;
								}
								else
								{
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
