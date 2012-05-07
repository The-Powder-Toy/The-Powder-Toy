#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_FIRE PT_FIRE 4
Element_FIRE::Element_FIRE()
{
    Identifier = "DEFAULT_PT_FIRE";
    Name = "FIRE";
    Colour = PIXPACK(0xFF1000);
    MenuVisible = 1;
    MenuSection = SC_EXPLOSIVE;
    Enabled = 1;
    
    Advection = 0.9f;
    AirDrag = 0.04f * CFDS;
    AirLoss = 0.97f;
    Loss = 0.20f;
    Collision = 0.0f;
    Gravity = -0.1f;
    Diffusion = 0.00f;
    HotAir = 0.001f  * CFDS;
    Falldown = 1;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 2;
    
    Temperature = R_TEMP+400.0f+273.15f;
    HeatConduct = 88;
    Description = "Ignites flammable materials. Heats air.";
    
    State = ST_GAS;
    Properties = TYPE_GAS|PROP_LIFE_DEC|PROP_LIFE_KILL;
    
    LowPressure = IPL;
    LowPressureTransition = NT;
    HighPressure = IPH;
    HighPressureTransition = NT;
    LowTemperature = ITL;
    LowTemperatureTransition = NT;
    HighTemperature = 2773.0f;
    HighTemperatureTransition = PT_PLSM;
    
    Update = &Element_FIRE::update;
    Graphics = &Element_FIRE::graphics;
}

//#TPT-Directive ElementHeader Element_FIRE static int update(UPDATE_FUNC_ARGS)
int Element_FIRE::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, rt, t = parts[i].type;
	if (t==PT_PLSM&&parts[i].ctype == PT_NBLE&&parts[i].life <=1)
	{
		t = PT_NBLE;
		sim->part_change_type(i,x,y,t);
		parts[i].life = 0;
	}
	if(t==PT_FIRE && parts[i].life <=1)
	{
		if (parts[i].tmp==3){
			t = PT_DSTW;
			sim->part_change_type(i,x,y,t);
			parts[i].life = 0;
			parts[i].ctype = PT_FIRE;
		}
		else if (parts[i].temp<625)
		{
			t = PT_SMKE;
			sim->part_change_type(i,x,y,t);
			parts[i].life = rand()%20+250;
		}
	}
	if(t==PT_PLSM && parts[i].life <=1)
	{
		if (parts[i].tmp==3){
			t = PT_DSTW;
			sim->part_change_type(i,x,y,t);
			parts[i].life = 0;
			parts[i].ctype = PT_FIRE;
		}
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->bmap[(y+ry)/CELL][(x+rx)/CELL] && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_STREAM)
					continue;
				rt = parts[r>>8].type;
				if ((surround_space || sim->elements[rt].Explosive) &&
					(t!=PT_SPRK || (rt!=PT_RBDM && rt!=PT_LRBD && rt!=PT_INSL)) &&
					(t!=PT_PHOT || rt!=PT_INSL) &&
				    (rt!=PT_SPNG || parts[r>>8].life==0) &&
				    (rt!=PT_H2 || (parts[r>>8].temp < 2273.15 && sim->pv[y/CELL][x/CELL] < 50.0f)) &&
				    sim->elements[rt].Flammable && (sim->elements[rt].Flammable + (int)(sim->pv[(y+ry)/CELL][(x+rx)/CELL]*10.0f))>(rand()%1000))
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_FIRE);
					parts[r>>8].temp = restrict_flt(sim->elements[PT_FIRE].Temperature + (sim->elements[rt].Flammable/2), MIN_TEMP, MAX_TEMP);
					parts[r>>8].life = rand()%80+180;
					parts[r>>8].tmp = parts[r>>8].ctype = 0;
					if (sim->elements[rt].Explosive)
						sim->pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
			}
	if (sim->legacy_enable) updateLegacy(UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

//#TPT-Directive ElementHeader Element_FIRE static int updateLegacy(UPDATE_FUNC_ARGS)
int Element_FIRE::updateLegacy(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt, lpv, t = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (sim->bmap[(y+ry)/CELL][(x+rx)/CELL] && sim->bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_STREAM)
					continue;
				rt = r&0xFF;
				lpv = (int)sim->pv[(y+ry)/CELL][(x+rx)/CELL];
				if (lpv < 1) lpv = 1;
				if (t!=PT_SPRK && sim->elements[rt].Meltable  && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) &&
						sim->elements[rt].Meltable*lpv>(rand()%1000))
				{
					if (t!=PT_LAVA || parts[i].life>0)
					{
						parts[r>>8].ctype = (rt==PT_BRMT)?PT_BMTL:parts[r>>8].type;
						parts[r>>8].ctype = (parts[r>>8].ctype==PT_SAND)?PT_GLAS:parts[r>>8].ctype;
						sim->part_change_type(r>>8,x+rx,y+ry,PT_LAVA);
						parts[r>>8].life = rand()%120+240;
					}
					else
					{
						parts[i].life = 0;
						t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
						parts[i].ctype = PT_NONE;//rt;
						sim->part_change_type(i,x,y,t);
						return 1;
					}
				}
				if (t!=PT_SPRK && (rt==PT_ICEI || rt==PT_SNOW))
				{
					parts[r>>8].type = PT_WATR;
					if (t==PT_FIRE)
					{
						sim->kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						t = parts[i].type = PT_STNE;
						sim->part_change_type(i,x,y,t);
					}
				}
				if (t!=PT_SPRK && (rt==PT_WATR || rt==PT_DSTW || rt==PT_SLTW))
				{
					sim->kill_part(r>>8);
					if (t==PT_FIRE)
					{
						sim->kill_part(i);
						return 1;
					}
					if (t==PT_LAVA)
					{
						parts[i].life = 0;
						t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
						parts[i].ctype = PT_NONE;
						sim->part_change_type(i,x,y,t);
					}
				}
			}
	return 0;
}


//#TPT-Directive ElementHeader Element_FIRE static int graphics(GRAPHICS_FUNC_ARGS)
int Element_FIRE::graphics(GRAPHICS_FUNC_ARGS)

{
	int caddress = restrict_flt(restrict_flt((float)cpart->life, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
	*colr = (unsigned char)ren->flm_data[caddress];
	*colg = (unsigned char)ren->flm_data[caddress+1];
	*colb = (unsigned char)ren->flm_data[caddress+2];
	
	*firea = 255;
	*firer = *colr;
	*fireg = *colg;
	*fireb = *colb;
	
	*pixel_mode = PMODE_NONE; //Clear default, don't draw pixel
	*pixel_mode |= FIRE_ADD;
	//Returning 0 means dynamic, do not cache
	return 0;
}

Element_FIRE::~Element_FIRE() {}