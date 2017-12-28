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
	PhotonReflectWavelengths = 0x00000000;

	Weight = 100;

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 251;
	Description = "Electricity. The basis of all electronics in TPT, travels along wires and other conductive elements.";

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
	int r, rx, ry, nearp, pavg, ct = parts[i].ctype, sender, receiver;
	Element_FIRE::update(UPDATE_FUNC_SUBCALL_ARGS);

	if (parts[i].life<=0)
	{
		if (ct==PT_WATR||ct==PT_SLTW||ct==PT_PSCN||ct==PT_NSCN||ct==PT_ETRD||ct==PT_INWR)
			parts[i].temp = R_TEMP + 273.15f;
		if (ct<=0 || ct>=PT_NUM || !sim->elements[parts[i].ctype].Enabled)
			ct = PT_METL;
		parts[i].ctype = PT_NONE;
		parts[i].life = 4;
		if (ct == PT_WATR)
			parts[i].life = 64;
		else if (ct == PT_SLTW)
			parts[i].life = 54;
		else if (ct == PT_SWCH)
			parts[i].life = 14;
		sim->part_change_type(i,x,y,ct);
		return 0;
	}
	//Some functions of SPRK based on ctype (what it is on)
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
			nearp = Element_ETRD::nearestSparkablePart(sim, i);
			if (nearp!=-1 && sim->parts_avg(i, nearp, PT_INSL)!=PT_INSL)
			{
				sim->CreateLine(x, y, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), PT_PLSM);
				parts[i].life = 20;
				sim->part_change_type(i,x,y,ct);
				ct = parts[i].ctype = PT_NONE;
				sim->part_change_type(nearp,(int)(parts[nearp].x+0.5f),(int)(parts[nearp].y+0.5f),PT_SPRK);
				parts[nearp].life = 9;
				parts[nearp].ctype = PT_ETRD;
			}
		}
		break;
	case PT_NBLE:
		if (parts[i].life<=1 && !(parts[i].tmp&0x1))
		{
			parts[i].life = rand()%150+50;
			sim->part_change_type(i,x,y,PT_PLSM);
			parts[i].ctype = PT_NBLE;
			if (parts[i].temp > 5273.15)
				parts[i].tmp |= 0x4;
			parts[i].temp = 3500;
			sim->pv[y/CELL][x/CELL] += 1;
		}
		break;
	case PT_TESC:
		if (parts[i].tmp>300)
			parts[i].tmp=300;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK && (rx || ry))
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
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_DSTW || TYP(r)==PT_SLTW || TYP(r)==PT_WATR)
					{
						int rnd = rand()%100;
						if (!rnd)
							sim->part_change_type(ID(r),x+rx,y+ry,PT_O2);
						else if (3>rnd)
							sim->part_change_type(ID(r),x+rx,y+ry,PT_H2);
					}
				}
		break;
	case PT_TUNG:
		if(parts[i].temp < 3595.0){
			parts[i].temp += (rand()%20)-4;
		}
	default:
		break;
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				receiver = TYP(r);
				sender = ct;
				pavg = sim->parts_avg(ID(r), i,PT_INSL);
				//receiver is the element SPRK is trying to conduct to
				//sender is the element the SPRK is on
				//First, some checks usually for (de)activation of elements
				switch (receiver)
				{
				case PT_SWCH:
					if (pavg!=PT_INSL && parts[i].life<4)
					{
						if(sender==PT_PSCN && parts[ID(r)].life<10) {
							parts[ID(r)].life = 10;
						}
						else if (sender==PT_NSCN)
						{
							parts[ID(r)].ctype = PT_NONE;
							parts[ID(r)].life = 9;
						}
					}
					break;
				case PT_SPRK:
					if (pavg!=PT_INSL && parts[i].life<4)
					{
						if (parts[ID(r)].ctype==PT_SWCH)
						{
							if (sender==PT_NSCN)
							{
								sim->part_change_type(ID(r),x+rx,y+ry,PT_SWCH);
								parts[ID(r)].ctype = PT_NONE;
								parts[ID(r)].life = 9;
							}
						}
						else if(parts[ID(r)].ctype==PT_NTCT||parts[ID(r)].ctype==PT_PTCT)		
							if (sender==PT_METL)
							{
								parts[ID(r)].temp = 473.0f;
							}
					}
					continue;
				case PT_PUMP: case PT_GPMP: case PT_HSWC: case PT_PBCN:
					if (parts[i].life<4)// PROP_PTOGGLE, Maybe? We seem to use 2 different methods for handling actived elements, this one seems better. Yes, use this one for new elements, PCLN is different for compatibility with existing saves
					{
						if (sender==PT_PSCN) parts[ID(r)].life = 10;
						else if (sender==PT_NSCN && parts[ID(r)].life>=10) parts[ID(r)].life = 9;
					}
					continue;
				case PT_LCRY:
					if (abs(rx)<2&&abs(ry)<2 && parts[i].life<4)
					{
						if (sender==PT_PSCN && parts[ID(r)].tmp == 0) parts[ID(r)].tmp = 2;
						else if (sender==PT_NSCN && parts[ID(r)].tmp == 3) parts[ID(r)].tmp = 1;
					}
					continue;
				case PT_PPIP:
					if (parts[i].life == 3 && pavg!=PT_INSL)
					{
						if (sender == PT_NSCN || sender == PT_PSCN || sender == PT_INST)
							Element_PPIP::flood_trigger(sim, x+rx, y+ry, sender);
					}
					continue;
				case PT_NTCT: case PT_PTCT: case PT_INWR:
					if (sender==PT_METL && pavg!=PT_INSL && parts[i].life<4)
					{
						parts[ID(r)].temp = 473.0f;
						if (receiver==PT_NTCT||receiver==PT_PTCT)
							continue;
					}
					break;
				case PT_EMP:
					if (!parts[ID(r)].life && parts[i].life > 0 && parts[i].life < 4)
					{
						sim->emp_trigger_count++;
						sim->emp_decor += 3;
						if (sim->emp_decor > 40)
							sim->emp_decor = 40;
						parts[ID(r)].life = 220;
					}
					continue;
				}

				if (pavg == PT_INSL) continue; //Insulation blocks everything past here
				if (!((sim->elements[receiver].Properties&PROP_CONDUCTS)||receiver==PT_INST||receiver==PT_QRTZ)) continue; //Stop non-conducting receivers, allow INST and QRTZ as special cases
				if (abs(rx)+abs(ry)>=4 &&sender!=PT_SWCH&&receiver!=PT_SWCH) continue; //Only switch conducts really far
				if (receiver==sender && receiver!=PT_INST && receiver!=PT_QRTZ) goto conduct; //Everything conducts to itself, except INST.

				//Sender cases, where elements can have specific outputs
				switch (sender)
				{
				case PT_INST:
					if (receiver==PT_NSCN)
						goto conduct;
					continue;
				case PT_SWCH:
					if (receiver==PT_PSCN||receiver==PT_NSCN||receiver==PT_WATR||receiver==PT_SLTW||receiver==PT_NTCT||receiver==PT_PTCT||receiver==PT_INWR)
						continue;
					break;
				case PT_ETRD:
					if (receiver==PT_METL||receiver==PT_BMTL||receiver==PT_BRMT||receiver==PT_LRBD||receiver==PT_RBDM||receiver==PT_PSCN||receiver==PT_NSCN)
						goto conduct;
					continue;
				case PT_NTCT:
					if (receiver==PT_PSCN || (receiver==PT_NSCN && parts[i].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (receiver==PT_PSCN || (receiver==PT_NSCN && parts[i].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (receiver==PT_NSCN || receiver==PT_PSCN)
						goto conduct;
					continue;
				default:
					break;
				}
				//Receiving cases, where elements can have specific inputs
				switch (receiver)
				{
				case PT_QRTZ:
					if ((sender==PT_NSCN||sender==PT_METL||sender==PT_PSCN||sender==PT_QRTZ) && (parts[ID(r)].temp<173.15||sim->pv[(y+ry)/CELL][(x+rx)/CELL]>8))
						goto conduct;
					continue;
				case PT_NTCT:
					if (sender==PT_NSCN || (sender==PT_PSCN&&parts[ID(r)].temp>373.0f))
						goto conduct;
					continue;
				case PT_PTCT:
					if (sender==PT_NSCN || (sender==PT_PSCN&&parts[ID(r)].temp<373.0f))
						goto conduct;
					continue;
				case PT_INWR:
					if (sender==PT_NSCN || sender==PT_PSCN)
						goto conduct;
					continue;
				case PT_INST:
					if (sender==PT_PSCN)
						goto conduct;
					continue;
				case PT_NBLE:
					if (!(parts[i].tmp&0x1))
						goto conduct;
					continue;
				case PT_PSCN:
					if (sender!=PT_NSCN)
						goto conduct;
					continue;
				default:
					break;
				}
			conduct:
				//Yay, passed normal conduction rules, check a few last things and change receiver to spark
				if (receiver==PT_WATR||receiver==PT_SLTW) {
					if (parts[ID(r)].life==0 && parts[i].life<3)
					{
						sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
						if (receiver==PT_WATR) parts[ID(r)].life = 6;
						else parts[ID(r)].life = 5;
						parts[ID(r)].ctype = receiver;
					}
				}
				else if (receiver==PT_INST) {
					if (parts[ID(r)].life==0 && parts[i].life<4)
					{
						sim->FloodINST(x+rx,y+ry,PT_SPRK,PT_INST);//spark the wire
					}
				}
				else if (parts[ID(r)].life==0 && parts[i].life<4) {
					parts[ID(r)].life = 4;
					parts[ID(r)].ctype = receiver;
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
					if (parts[ID(r)].temp+10.0f<673.0f&&!sim->legacy_enable&&(receiver==PT_METL||receiver==PT_BMTL||receiver==PT_BRMT||receiver==PT_PSCN||receiver==PT_NSCN||receiver==PT_ETRD||receiver==PT_NBLE||receiver==PT_IRON))
						parts[ID(r)].temp = parts[ID(r)].temp+10.0f;
				}
				else if (!parts[ID(r)].life && sender==PT_ETRD && parts[i].life==5) //ETRD is odd and conducts to others only at life 5, this could probably be somewhere else
				{
					sim->part_change_type(i,x,y,sender);
					parts[i].ctype = PT_NONE;
					parts[i].life = 20;
					parts[ID(r)].life = 4;
					parts[ID(r)].ctype = receiver;
					sim->part_change_type(ID(r),x+rx,y+ry,PT_SPRK);
				}
			}
	return 0;
}



//#TPT-Directive ElementHeader Element_SPRK static int graphics(GRAPHICS_FUNC_ARGS)
int Element_SPRK::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 60;
	*firer = *colr/2;
	*fireg = *colg/2;
	*fireb = *colb/2;
	*pixel_mode |= FIRE_SPARK;
	return 1;
}


Element_SPRK::~Element_SPRK() {}
