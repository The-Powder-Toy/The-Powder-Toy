#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_VIBR PT_VIBR 
Element_VIBR::Element_VIBR()
{
	Identifier = "DEFAULT_PT_VIBR";
	Name = "VIBR";
	Colour = PIXPACK(0x002900);
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.85f;
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Vibranium. Stores energy and releases it in violent explosions.";

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

	Update = &Element_VIBR::update;
	Graphics = &Element_VIBR::graphics;
}

int Element_VIBR::update(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt, transfer, trade;
	rt = parts[r>>8].type;
	//Heat absorption code
	if (parts[i].temp>274.65f)
	{
		parts[i].ctype++;
		parts[i].temp-=3;
	}
	if (parts[i].temp<271.65f)
	{
		parts[i].ctype--;
		parts[i].temp+=3;
	}
	//Pressure absorption code
	if (sim->pv[y/CELL][x/CELL]>2.5)
	{
		parts[i].tmp++;
		sim->pv[y/CELL][x/CELL]--;
	}
	if (sim->pv[y/CELL][x/CELL]<-2.5)
	{
		sim->pv[y/CELL][x/CELL]++;
	}
	//Release sparks before explode
	if (parts[i].ctype>3200)
	{
		parts[r>>8].life = 4;
		parts[r>>8].ctype = rt;
		sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
	}
	//Release all heat
	if ((1000<parts[i].ctype && parts[i].ctype<1200) || parts[i].tmp>90 || (80<parts[i].tmp2 && parts[i].tmp2<100))
	{
		for(rx=-1; rx<2; rx++)
			for(ry=-1; ry<2; ry++)
				if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && pmap[y+ry][x+rx] && (pmap[y+ry][x+rx]&0xFF)!=PT_VIBR&& (pmap[y+ry][x+rx]&0xFF)!=0xFF)
				{
					r = pmap[y+ry][x+rx];
					if(parts[r>>8].temp+ (parts[r>>8].temp*0.2f)<=MAX_TEMP)
					{
						parts[r>>8].temp += parts[r>>8].temp*0.2f;
					}
					else 
					{
						parts[r>>8].temp = MAX_TEMP;
					}
				}
				if (1200<parts[i].ctype || parts[i].tmp>95 || 90<parts[i].tmp2)
					parts[i].ctype--;
	}
	//Explosion code
	if (parts[i].ctype>1200 || parts[i].tmp>100 || parts[i].tmp2>100)
	{
		sim->part_change_type(i,x,y,PT_EXOT);
		sim->create_part(-1,x,y-1,PT_ELEC);
		sim->create_part(-1,x,y-1,PT_NEUT);
		sim->create_part(-1,x,y-1,PT_PHOT);
		sim->create_part(-1,x,y-1,PT_BREC);
		parts[i].temp=9000;
		sim->pv[y/CELL][x/CELL]=256;
	}
	//Neighbor check loop
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				//Melts into EXOT
				if ((r&0xFF)==PT_EXOT && !(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_EXOT);
				}
				//Absorbs energy particles
				if ((r&0xFF)==PT_NEUT || (r&0xFF)==PT_ELEC || (r&0xFF)==PT_PHOT)
				{
					parts[i].tmp2++;
					sim->kill_part(r>>8);
				}
				if ((r&0xFF)==PT_BOYL)
				{
					sim->part_change_type(i,x,y,PT_BVBR);
				}
			}
			for (trade = 0; trade < 9; trade++)
			{
				int random = rand();
				rx = random%7-3;
				ry = (random>>3)%7-3;
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r&0xFF)!=PT_VIBR)
						continue;
					if (parts[i].tmp > parts[r>>8].tmp) { //tmp diffusion
						transfer = parts[i].tmp - parts[r>>8].tmp;
						if (transfer == 1) {
							parts[r>>8].tmp++;
							parts[i].tmp--;
							trade = 9;
						} else if (transfer > 0) {
							parts[r>>8].tmp += transfer/2;
							parts[i].tmp -= transfer/2;
							trade = 9;
						}
					}
					if (parts[i].tmp2 > parts[r>>8].tmp2) { //tmp2 diffusion
						transfer = parts[i].tmp2 - parts[r>>8].tmp2;
						if (transfer == 1)
						{
							parts[r>>8].tmp2++;
							parts[i].tmp2--;
							trade = 9; }
						else if (transfer > 0) {
							parts[r>>8].tmp2 += transfer/2;
							parts[i].tmp2 -= transfer/2;
							trade = 9;
						}
					}
					if (parts[i].ctype > parts[r>>8].ctype) { //ctype diffusion
						transfer = parts[i].ctype - parts[r>>8].ctype;
						if (transfer == 1) {
							parts[r>>8].ctype++;
							parts[i].ctype--;
							trade = 9;
						} else if (transfer > 0) {
							parts[r>>8].ctype += transfer/2;
							parts[i].ctype -= transfer/2;
							trade = 9;
						}
					}
				}
			}	
			return 0;
}

int Element_VIBR::graphics(GRAPHICS_FUNC_ARGS)
{
	float maxtemp = fmax(cpart->tmp,cpart->temp);
	int gradient = max(cpart->ctype/12.0f, cpart->tmp);
	gradient = max(gradient, cpart->tmp2);
	if (gradient > 31 && gradient < 63)
	{
		*colr += (int)restrict_flt((gradient-31)*0.627,0,51);
		*colg += (int)restrict_flt((gradient-31)*0.582,0,55);
		*colb += (int)restrict_flt((gradient-31)*0.627,0,51);
	}
	else if (gradient >= 63 && gradient < 94)
	{
		*colr += (int)restrict_flt((gradient-63)*.636+51,51,100);
		*colg += (int)restrict_flt((gradient-63)*.969+55,55,87);
		*colb += (int)restrict_flt((gradient-63)*.636+51,51,100);
	}
	else if (gradient >= 94 && gradient < 100)
	{
		*colr += (int)restrict_flt((gradient-94)*19.7+100,100,218);
		*colg += (int)restrict_flt((gradient-94)*17.5+87,87,192);
		*colb += (int)restrict_flt((gradient-94)*19.7+100,100,218);
	}
	else if (gradient >= 100)
	{
		*pixel_mode = PMODE_NONE;
		*pixel_mode |= FIRE_BLEND;
		*firea = 90;
		*colr = 146;
		*colg = 158;
		*colb = 113;
		*firer = *colr;
		*fireg = *colg;
		*fireb = *colb;
	}

	return 0;
}

Element_VIBR::~Element_VIBR() {}