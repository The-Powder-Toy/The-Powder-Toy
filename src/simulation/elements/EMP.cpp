#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_EMP PT_EMP 134
Element_EMP::Element_EMP()
{
    Identifier = "DEFAULT_PT_EMP";
    Name = "EMP";
    Colour = PIXPACK(0x66AAFF);
    MenuVisible = 1;
    MenuSection = SC_ELEC;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.90f;
    Loss = 0.00f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.0f;
    HotAir = 0.0f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 3;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 121;
    Description = "Breaks activated electronics.";
    
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
    
    Update = &Element_EMP::update;
    Graphics = &Element_EMP::graphics;
}

//#TPT-Directive ElementHeader Element_EMP static int update(UPDATE_FUNC_ARGS)
int Element_EMP::update(UPDATE_FUNC_ARGS)
 {
	int r,rx,ry,ok=0,t,n,nx,ny;
	if (parts[i].life)
		return 0;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SPRK && parts[r>>8].life>0 && parts[r>>8].life<4)
				{
					ok=1;
					break;
				}
			}
	if (!ok)
		return 0;
	parts[i].life=220;
	sim->emp_decor += 3;
	if (sim->emp_decor > 40)
		sim->emp_decor = 40;
	for (r=0; r<=sim->parts_lastActiveIndex; r++)
	{
		t=parts[r].type;
		rx=parts[r].x;
		ry=parts[r].y;
		if (t==PT_SPRK || (t==PT_SWCH && parts[r].life!=0 && parts[r].life!=10) || (t==PT_WIRE && parts[r].ctype>0))
		{
			int is_elec=0;
			if ((parts[r].ctype==PT_PSCN || parts[r].ctype==PT_NSCN || parts[r].ctype==PT_PTCT ||
			        parts[r].ctype==PT_NTCT || parts[r].ctype==PT_INST || parts[r].ctype==PT_SWCH) || t==PT_WIRE || t==PT_SWCH)
			{
				is_elec=1;
				if (sim->elements[parts[r].type].HeatConduct && rand()%100==0)
					parts[r].temp = restrict_flt(parts[r].temp+3000.0f, MIN_TEMP, MAX_TEMP);
				if (rand()%80==0)
					sim->part_change_type(r, rx, ry, PT_BREC);
				else if (rand()%120==0)
					sim->part_change_type(r, rx, ry, PT_NTCT);
			}
			
			for (nx=-2; nx<3; nx++)
				for (ny=-2; ny<3; ny++)
					if (rx+nx>=0 && ry+ny>=0 && rx+nx<XRES && ry+ny<YRES && (rx || ry))
					{
						n = pmap[ry+ny][rx+nx];
						if (!n)
							continue;
						/*if ((n&0xFF)==PT_BTRY && rand()%60==0)
						{
							part_change_type(n>>8, rx+nx, ry+ny, PT_PLSM);
							parts[n>>8].life=rand()%100+70;
							parts[n>>8].temp+=3000;
						}*/

						//Some elements should only be affected by wire/swch, or by a spark on inst/semiconductor
						//So not affected by spark on metl, watr etc
						if (is_elec)
						{
							if (((n&0xFF)==PT_METL || (n&0xFF)==PT_BMTL) && rand()%280==0)
							{
								parts[n>>8].temp = restrict_flt(parts[n>>8].temp+3000.0f, MIN_TEMP, MAX_TEMP);
							}
							if ((n&0xFF)==PT_BMTL && rand()%160==0)
							{
								sim->part_change_type(n>>8, rx+nx, ry+ny, PT_BMTL);//TODO: Redundant, was this meant to be BRMT or something?
								parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
							}
							if ((n&0xFF)==PT_METL && rand()%300==0)
							{
								sim->part_change_type(n>>8, rx+nx, ry+ny, PT_BMTL);
							}
							if ((n&0xFF)==PT_WIFI && rand()%8==0)
							{
								//Randomise channel
								parts[n>>8].temp = rand()%MAX_TEMP;
							}
							if ((n&0xFF)==PT_WIFI && rand()%16==0)
							{
								sim->create_part(n>>8, rx+nx, ry+ny, PT_BREC);
								parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
							}
						}
						if ((n&0xFF)==PT_SWCH && rand()%100==0)
						{
							sim->part_change_type(n>>8, rx+nx, ry+ny, PT_BREC);
						}
						if ((n&0xFF)==PT_SWCH && rand()%100==0)
						{
							parts[n>>8].temp = restrict_flt(parts[n>>8].temp+2000.0f, MIN_TEMP, MAX_TEMP);
						}
						if ((n&0xFF)==PT_ARAY && rand()%60==0)
						{
							sim->create_part(n>>8, rx+nx, ry+ny, PT_BREC);
							parts[n>>8].temp = restrict_flt(parts[n>>8].temp+1000.0f, MIN_TEMP, MAX_TEMP);
						}
						if (t==PT_DLAY && rand()%70==0)
						{
							//Randomise delay
							parts[n>>8].temp = (rand()%256) + 273.15f;
						}
					}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_EMP static int graphics(GRAPHICS_FUNC_ARGS)
int Element_EMP::graphics(GRAPHICS_FUNC_ARGS)

{
	if(cpart->life)
	{
		*colr = cpart->life*1.5;
		*colg = cpart->life*1.5;
		*colb = 200-(cpart->life);
		if (*colr>255)
			*colr = 255;
		if (*colg>255)
			*colg = 255;
		if (*colb>255)
			*colb = 255;
		if (*colb<=0)
			*colb = 0;
	}
	return 0;
}


Element_EMP::~Element_EMP() {}