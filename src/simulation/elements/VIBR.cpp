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
	int r, rx, ry, transfer, trade;
	if (!parts[i].life)
	{
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
	}
	//Release sparks before explode
	if (parts[i].life && parts[i].life < 300)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		r = pmap[y+ry][x+rx];
		if ((r&0xFF) && (r&0xFF) != PT_BREC && (sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && !parts[r>>8].life)
		{
			parts[r>>8].life = 4;
			parts[r>>8].ctype = r>>8;
			sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
		}
	}
	//Release all heat
	if (!parts[i].life && (parts[i].ctype > 1200 || parts[i].tmp > 100 || parts[i].tmp2 > 100))
		parts[i].life = 750;
	if (parts[i].life && parts[i].life < 500)
	{
		int random = rand();
		rx = random%7-3;
		ry = (random>>3)%7-3;
		if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES)
		{
			r = pmap[y+ry][x+rx];
			if ((r&0xFF) && (r&0xFF)!=PT_VIBR)
			{
				parts[r>>8].temp += parts[i].ctype*6;
				parts[i].ctype -= parts[i].ctype*2;
			}
		}
	}
	//Explosion code
	if (parts[i].life == 1)
	{
		sim->part_change_type(i,x,y,PT_EXOT);
		int random = rand(), index;
		index = sim->create_part(-3,x+(random&3)-1,y+((random>>2)&3)-1,PT_ELEC);
		if (index != -1)
			parts[index].temp = 7000;
		index = sim->create_part(-3,x+((random>>4)&3)-1,y+((random>>6)&3)-1,PT_NEUT);
		if (index != -1)
			parts[index].temp = 7000;
		index = sim->create_part(-3,x+((random>>8)&3)-1,y+((random>>10)&3)-1,PT_PHOT);
		if (index != -1)
			parts[index].temp = 7000;
		index = sim->create_part(-3,x+((random>>12)&3)-1,y+rand()%3-1,PT_BREC);
		if (index != -1)
			parts[index].temp = 7000;
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
	if (gradient >= 100 || cpart->life)
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
	else if (gradient >= 94 && gradient < 100)
	{
		*colr += (int)restrict_flt((gradient-94)*19.7+100,100,218);
		*colg += (int)restrict_flt((gradient-94)*17.5+87,87,192);
		*colb += (int)restrict_flt((gradient-94)*19.7+100,100,218);
	}
	else if (gradient >= 63 && gradient < 94)
	{
		*colr += (int)restrict_flt((gradient-63)*1.58+51,51,100);
		*colg += (int)restrict_flt((gradient-63)*1.03+55,55,87);
		*colb += (int)restrict_flt((gradient-63)*1.58+51,51,100);
	}
	else if (gradient > 31 && gradient < 63)
	{
		*colr += (int)restrict_flt((gradient-31)*1.59,0,51);
		*colg += (int)restrict_flt((gradient-31)*1.72,0,55);
		*colb += (int)restrict_flt((gradient-31)*1.59,0,51);
	}
	return 0;
}

Element_VIBR::~Element_VIBR() {}