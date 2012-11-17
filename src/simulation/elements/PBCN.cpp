#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_PBCN PT_PBCN 153
Element_PBCN::Element_PBCN()
{
    Identifier = "DEFAULT_PT_PBCN";
    Name = "PBCN";
    Colour = PIXPACK(0x3B1D0A);
    MenuVisible = 1;
    MenuSection = SC_POWERED;
    Enabled = 1;
    
    Advection = 0.0f;
    AirDrag = 0.00f * CFDS;
    AirLoss = 0.97f;
    Loss = 0.50f;
    Collision = 0.0f;
    Gravity = 0.0f;
    Diffusion = 0.00f;
    HotAir = 0.000f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 12;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 251;
    Description = "Powered breakable clone";
    
    State = ST_NONE;
    Properties = TYPE_SOLID;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = ITH;
    HighTemperatureTransition = NT;
    
    Update = &Element_PBCN::update;
    Graphics = &Element_PBCN::graphics;
}

//#TPT-Directive ElementHeader Element_PBCN static int update(UPDATE_FUNC_ARGS)
int Element_PBCN::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	if (!parts[i].tmp2 && sim->pv[y/CELL][x/CELL]>4.0f)
		parts[i].tmp2 = rand()%40+80;
	if (parts[i].tmp2)
	{
		float advection = 0.1f;
		parts[i].vx += advection*sim->vx[y/CELL][x/CELL];
		parts[i].vy += advection*sim->vy[y/CELL][x/CELL];
		parts[i].tmp2--;
		if(!parts[i].tmp2){
			sim->kill_part(i);
			return 1;
		}
	}
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOLALT)))
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN &&
				        (r&0xFF)!=PT_BCLN &&  (r&0xFF)!=PT_SPRK &&
				        (r&0xFF)!=PT_NSCN && (r&0xFF)!=PT_PSCN &&
				        (r&0xFF)!=PT_STKM && (r&0xFF)!=PT_STKM2 &&
				        (r&0xFF)!=PT_PBCN && (r&0xFF)<PT_NUM)
					{
						parts[i].ctype = r&0xFF;
						if ((r&0xFF)==PT_LIFE || (r&0xFF)==PT_LAVA)
							parts[i].tmp = parts[r>>8].ctype;
					}
				}
	if (parts[i].life==10)
	{
		
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_PBCN)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
	if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && sim->elements[parts[i].ctype].Enabled && parts[i].life==10) {
		if (parts[i].ctype==PT_PHOT) {//create photons a different way
			for (rx=-1; rx<2; rx++)
			{
				for (ry = -1; ry < 2; ry++)
				{
					if (rx || ry)
					{
						int r = sim->create_part(-1, x + rx, y + ry,
								parts[i].ctype);
						if (r != -1)
						{
							parts[r].vx = rx * 3;
							parts[r].vy = ry * 3;
							if (r>i)
							{
								// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
								parts[r].flags |= FLAG_SKIPMOVE;
							}
						}
					}
				}
			}
		}
		else if (parts[i].ctype==PT_LIFE) {//create life a different way
			for (rx=-1; rx<2; rx++) {
				for (ry=-1; ry<2; ry++) {
					sim->create_part(-1, x+rx, y+ry, parts[i].ctype|(parts[i].tmp<<8));
				}
			}
		}
		else if (parts[i].ctype!=PT_LIGH || (rand()%30)==0)
		{
			int np = sim->create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
			if (np>=0)
			{
				if (parts[i].ctype==PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && sim->elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA)
					parts[np].ctype = parts[i].tmp;
			}
		}
	}
	return 0;
}


//#TPT-Directive ElementHeader Element_PBCN static int graphics(GRAPHICS_FUNC_ARGS)
int Element_PBCN::graphics(GRAPHICS_FUNC_ARGS)

{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod/2;
	return 0;
}


Element_PBCN::~Element_PBCN() {}
