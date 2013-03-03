#include "simulation/Elements.h"
//#TPT-Directive ElementClass Element_SPRK PT_SPRK 15
Element_SPRK::Element_SPRK()
{
    Identifier = "DEFAULT_PT_SPRK";
    Name = "SPRK";
    Colour = PIXPACK(0xFFFF80);
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
    HotAir = 0.001f	* CFDS;
    Falldown = 0;
    
    Flammable = 0;
    Explosive = 0;
    Meltable = 0;
    Hardness = 1;
    
    Weight = 100;
    
    Temperature = R_TEMP+0.0f	+273.15f;
    HeatConduct = 251;
    Description = "Electricity. Conducted by metal and water.";
    
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
    
    Update = &Element_SPRK::update;
    Graphics = &Element_SPRK::graphics;
}

//#TPT-Directive ElementHeader Element_SPRK static int update(UPDATE_FUNC_ARGS)
int Element_SPRK::update(UPDATE_FUNC_ARGS)
 {
	int r, rx, ry, rt, conduct_sprk, nearp, pavg, ct = parts[i].ctype;
	Element_FIRE::update(UPDATE_FUNC_SUBCALL_ARGS);

	if (parts[i].life<=0)
	{
		if (ct==PT_WATR||ct==PT_SLTW||ct==PT_PSCN||ct==PT_NSCN||ct==PT_ETRD||ct==PT_INWR)
			parts[i].temp = R_TEMP + 273.15f;
		if (ct<=0 || ct>=PT_NUM || !sim->elements[parts[i].ctype].Enabled)
			ct = PT_METL;
		sim->part_change_type(i,x,y,ct);
		parts[i].ctype = PT_NONE;
		parts[i].life = 4;
		if (ct == PT_WATR)
			parts[i].life = 64;
		else if (ct == PT_SLTW)
			parts[i].life = 54;
		else if (ct == PT_SWCH)
			parts[i].life = 14;
		return 0;
	}
	switch(ct)
	{
	case PT_SPRK:
		sim->kill_part(i);
		return 1;
	case PT_NTCT:
	case PT_PTCT:
		Element_NTCT::update(UPDATE_FUNC_SUBCALL_ARGS);
		break;
	case PT_ETRD:
		if (parts[i].life==1)
		{
			nearp = sim->nearest_part(i, PT_ETRD, -1);
			if (nearp!=-1 && sim->parts_avg(i, nearp, PT_INSL)!=PT_INSL)
			{
				sim->CreateLine(x, y, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), 0, 0, PT_PLSM, 0);
				sim->part_change_type(i,x,y,ct);
				ct = parts[i].ctype = PT_NONE;
				parts[i].life = 20;
				sim->part_change_type(nearp,(int)(parts[nearp].x+0.5f),(int)(parts[nearp].y+0.5f),PT_SPRK);
				parts[nearp].life = 9;
				parts[nearp].ctype = PT_ETRD;
			}
		}
		break;
	case PT_NBLE:
		if (parts[i].life<=1&&parts[i].tmp!=1)
		{
			parts[i].life = rand()%150+50;
			sim->part_change_type(i,x,y,PT_PLSM);
			parts[i].ctype = PT_NBLE;
			if (parts[i].temp > 5273.15)
				parts[i].tmp |= 4;
			parts[i].temp = 3500;
			sim->pv[y/CELL][x/CELL] += 1;
		}
		break;
	case PT_TESC:
		if (parts[i].tmp>300)
			parts[i].tmp=300;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (r)
						continue;
					if (parts[i].tmp>4 && rand()%(parts[i].tmp*parts[i].tmp/20+6)==0)
					{
						int p = sim->create_part(-1, x+rx*2, y+ry*2, PT_LIGH);
						if (p!=-1)
						{
							parts[p].life=rand()%(2+parts[i].tmp/15)+parts[i].tmp/7;
							if (parts[i].life>60)
								parts[i].life=60;
							parts[p].temp=parts[p].life*parts[i].tmp/2.5;
							parts[p].tmp2=1;
							parts[p].tmp=atan2(-ry, (float)rx)/M_PI*360;
							parts[i].temp-=parts[i].tmp*2+parts[i].temp/5; // slight self-cooling
							if (fabs(sim->pv[y/CELL][x/CELL])!=0.0f)
							{
								if (fabs(sim->pv[y/CELL][x/CELL])<=0.5f)
									sim->pv[y/CELL][x/CELL]=0;
								else
									sim->pv[y/CELL][x/CELL]-=(sim->pv[y/CELL][x/CELL]>0)?0.5:-0.5;
							}
						}
					}
				}
		break;
	case PT_IRON:
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if ((r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW || (r&0xFF)==PT_WATR)
					{
						int rnd = rand()%100;
						if (!rnd)
							sim->part_change_type(r>>8,x+rx,y+ry,PT_O2);
						else if (3>rnd)
							sim->part_change_type(r>>8,x+rx,y+ry,PT_H2);
					}
				}
		break;
	default:
		break;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				conduct_sprk = 1;
// ct = spark from material, rt = spark to material. Make conduct_sprk = 0 if conduction not allowed
				pavg = sim->parts_avg(r>>8, i,PT_INSL);
				switch (rt)
				{
				case PT_SWCH:
					if (pavg!=PT_INSL && parts[i].life<4)
					{
						if(ct==PT_PSCN && parts[r>>8].life<10) {
							parts[r>>8].life = 10;
						}
						else if (ct==PT_NSCN)
						{
							parts[r>>8].ctype = PT_NONE;
							parts[r>>8].life = 9;
						}
					}
					break;
				case PT_SPRK:
					if (pavg!=PT_INSL && parts[i].life<4)
					{
						if (parts[r>>8].ctype==PT_SWCH)
						{
							if (ct==PT_NSCN)
							{
								sim->part_change_type(r>>8,x+rx,y+ry,PT_SWCH);
								parts[r>>8].ctype = PT_NONE;
								parts[r>>8].life = 9;
							}
						}
						else if(parts[r>>8].ctype==PT_NTCT||parts[r>>8].ctype==PT_PTCT)		
							if (ct==PT_METL)
							{
								parts[r>>8].temp = 473.0f;
							}
					}
					continue;
				case PT_PUMP:
				case PT_GPMP:
				case PT_HSWC:
				case PT_PBCN:
					if (parts[i].life<4)// PROP_PTOGGLE, Maybe? We seem to use 2 different methods for handling actived elements, this one seems better. Yes, use this one for new elements, PCLN is different for compatibility with existing saves
					{
						if (ct==PT_PSCN) parts[r>>8].life = 10;
						else if (ct==PT_NSCN && parts[r>>8].life>=10) parts[r>>8].life = 9;
					}
					continue;
				case PT_LCRY:
					if (abs(rx)<2&&abs(ry)<2 && parts[i].life<4)
					{
						if (ct==PT_PSCN && parts[r>>8].tmp == 0) parts[r>>8].tmp = 2;
						else if (ct==PT_NSCN && parts[r>>8].tmp == 3) parts[r>>8].tmp = 1;
					}
					continue;
				case PT_PPIP:
					if (parts[i].life == 3 && pavg!=PT_INSL)
					{
						if (ct == PT_NSCN || ct == PT_PSCN || ct == PT_INST)
							Element_PPIP::flood_trigger(sim, x+rx, y+ry, ct);
					}
					continue;
				case PT_NTCT: case PT_PTCT: case PT_INWR:
					if (ct==PT_METL && pavg!=PT_INSL && parts[i].life<4)
					{
						parts[r>>8].temp = 473.0f;
						if (rt==PT_NTCT||rt==PT_PTCT)
							continue;
					}
					break;
				}
				//the crazy conduct checks
				if (pavg == PT_INSL) continue;
				if (!((sim->elements[rt].Properties&PROP_CONDUCTS)||rt==PT_INST||rt==PT_QRTZ)) continue;
				if (abs(rx)+abs(ry)>=4 &&ct!=PT_SWCH&&rt!=PT_SWCH)
					continue;
				if (rt==ct && rt!=PT_INST) goto conduct;

				switch (ct)
				{
				case PT_INST:
					if (rt==PT_NSCN)
						goto conduct;
					continue;
				case PT_SWCH:
					if (rt==PT_PSCN||rt==PT_NSCN||rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR)
						continue;
					goto conduct;
				case PT_ETRD:
					if (rt==PT_METL||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN)
						goto conduct;
					continue;
				case PT_NTCT:
					if (rt==PT_PSCN || (rt==PT_NSCN && parts[i].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (rt==PT_PSCN || (rt==PT_NSCN && parts[i].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (rt==PT_NSCN || rt==PT_PSCN)
						goto conduct;
					continue;
				}
				switch (rt)
				{
				case PT_QRTZ:
					if ((ct==PT_NSCN||ct==PT_METL||ct==PT_PSCN||ct==PT_QRTZ) && (parts[r>>8].temp<173.15||sim->pv[(y+ry)/CELL][(x+rx)/CELL]>8))
						goto conduct;
					continue;
				case PT_NTCT:
					if (ct==PT_NSCN || (ct==PT_PSCN&&parts[r>>8].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (ct==PT_NSCN || (ct==PT_PSCN&&parts[r>>8].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (ct==PT_NSCN || ct==PT_PSCN)
						goto conduct;
					continue;
				case PT_INST:
					if (ct==PT_PSCN)
						goto conduct;
					continue;
				case PT_NBLE:
					if (parts[r>>8].tmp != 1)
						goto conduct;
					continue;
				case PT_PSCN:
					if (ct!=PT_NSCN) 
						goto conduct;
					continue;
				}
			conduct:
				if (rt==PT_WATR||rt==PT_SLTW) {
					if (parts[r>>8].life==0 && parts[i].life<3)
					{
						sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
						if (rt==PT_WATR) parts[r>>8].life = 6;
						else parts[r>>8].life = 5;
						parts[r>>8].ctype = rt;
					}
				}
				else if (rt==PT_INST) {
					if (parts[r>>8].life==0 && parts[i].life<4)
					{
						sim->FloodINST(x+rx,y+ry,PT_SPRK,PT_INST);//spark the wire
					}
				}
				else if (parts[r>>8].life==0 && parts[i].life<4) {
					parts[r>>8].life = 4;
					parts[r>>8].ctype = rt;
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
					if (parts[r>>8].temp+10.0f<673.0f&&!sim->legacy_enable&&(rt==PT_METL||rt==PT_BMTL||rt==PT_BRMT||rt==PT_PSCN||rt==PT_NSCN||rt==PT_ETRD||rt==PT_NBLE||rt==PT_IRON))
						parts[r>>8].temp = parts[r>>8].temp+10.0f;
				}
				else if (ct==PT_ETRD && parts[i].life==5)
				{
					sim->part_change_type(i,x,y,ct);
					parts[i].ctype = PT_NONE;
					parts[i].life = 20;
					parts[r>>8].life = 4;
					parts[r>>8].ctype = rt;
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SPRK);
				}
				
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_SPRK static int graphics(GRAPHICS_FUNC_ARGS)
int Element_SPRK::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 80;
	*firer = 170;
	*fireg = 200;
	*fireb = 220;
	*pixel_mode |= FIRE_ADD;
	return 1;
}


Element_SPRK::~Element_SPRK() {}
