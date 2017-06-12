#include "simulation/Elements.h"

//#TPT-Directive ElementClass Element_LIGH PT_LIGH 87
Element_LIGH::Element_LIGH()
{
	Identifier = "DEFAULT_PT_LIGH";
	Name = "LIGH";
	Colour = PIXPACK(0xFFFFC0);
	MenuVisible = 1;
	MenuSection = SC_EXPLOSIVE;
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

	Temperature = R_TEMP+0.0f	+273.15f;
	HeatConduct = 0;
	Description = "Lightning. Change the brush size to set the size of the lightning.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &Element_LIGH::update;
	Graphics = &Element_LIGH::graphics;
}

#define LIGHTING_POWER 0.65

//#TPT-Directive ElementHeader Element_LIGH static int update(UPDATE_FUNC_ARGS)
int Element_LIGH::update(UPDATE_FUNC_ARGS)

{
	/*
	 *
	 * tmp2:
	 * -1 - part will be removed
	 * 0 - "branches" of the lightning
	 * 1 - bending
	 * 2 - branching
	 * 3 - transfer spark or make destruction
	 * 4 - first pixel
	 *
	 * life - "thickness" of lighting (but anyway one pixel)
	 *
	 * tmp - angle of lighting, measured in degrees anticlockwise from the positive x direction
	 *
	*/
	int r,rx,ry,rt, multipler, powderful;
	float angle, angle2=-1;
	powderful = parts[i].temp*(1+parts[i].life/40)*LIGHTING_POWER;
	//Element_FIRE::update(UPDATE_FUNC_SUBCALL_ARGS);
	if (sim->aheat_enable)
	{
		sim->hv[y/CELL][x/CELL]+=powderful/50;
		if (sim->hv[y/CELL][x/CELL]>MAX_TEMP)
			sim->hv[y/CELL][x/CELL]=MAX_TEMP;
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (BOUNDS_CHECK && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				rt = r&0xFF;
				if ((surround_space || sim->elements[rt].Explosive) &&
				    (rt!=PT_SPNG || parts[r>>8].life==0) &&
					sim->elements[rt].Flammable && (sim->elements[rt].Flammable + (int)(sim->pv[(y+ry)/CELL][(x+rx)/CELL]*10.0f))>(rand()%1000))
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_FIRE);
					parts[r>>8].temp = restrict_flt(sim->elements[PT_FIRE].Temperature + (sim->elements[rt].Flammable/2), MIN_TEMP, MAX_TEMP);
					parts[r>>8].life = rand()%80+180;
					parts[r>>8].tmp = parts[r>>8].ctype = 0;
					if (sim->elements[rt].Explosive)
						sim->pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
				switch (rt)
				{
				case PT_LIGH:
				case PT_TESC:
					continue;
				case PT_CLNE:
				case PT_THDR:
				case PT_DMND:
				case PT_FIRE:
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					continue;
				case PT_DEUT:
				case PT_PLUT:
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful, MIN_TEMP, MAX_TEMP);
					sim->pv[y/CELL][x/CELL] +=powderful/35;
					if (!(rand()%3))
					{
						sim->part_change_type(r>>8,x+rx,y+ry,PT_NEUT);
						parts[r>>8].life = rand()%480+480;
						parts[r>>8].vx=rand()%10-5;
						parts[r>>8].vy=rand()%10-5;
					}
					break;
				case PT_COAL:
				case PT_BCOL:
					if (parts[r>>8].life>100)
						parts[r>>8].life = 99;
					break;
				case PT_STKM:
					if (sim->player.elem!=PT_LIGH)
						parts[r>>8].life-=powderful/100;
					break;
				case PT_STKM2:
					if (sim->player2.elem!=PT_LIGH)
						parts[r>>8].life-=powderful/100;
					break;
				case PT_HEAC:
					parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					if (parts[r>>8].temp > sim->elements[PT_HEAC].HighTemperature)
					{
						sim->part_change_type(r>>8, x+rx, y+ry, PT_LAVA);
						parts[r>>8].ctype = PT_HEAC;
					}
					break;
				default:
					break;
				}
				if ((sim->elements[r&0xFF].Properties&PROP_CONDUCTS) && parts[r>>8].life==0)
					sim->create_part(r>>8,x+rx,y+ry,PT_SPRK);
				sim->pv[y/CELL][x/CELL] += powderful/400;
				if (sim->elements[r&0xFF].HeatConduct) parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/1.3, MIN_TEMP, MAX_TEMP);
			}
	if (parts[i].tmp2==3)
	{
		parts[i].tmp2=0;
		return 1;
	}
	else if (parts[i].tmp2<=-1)
	{
		sim->kill_part(i);
		return 1;
	}
	else if (parts[i].tmp2<=0 || parts[i].life<=1)
	{
		if (parts[i].tmp2>0)
			parts[i].tmp2=0;
		parts[i].tmp2--;
		return 1;
	}

	//Completely broken and laggy function, possibly can be fixed later
	/*int pNear = LIGH_nearest_part(sim, i, parts[i].life*2.5);
	if (pNear!=-1)
	{
		int t=parts[pNear].type;
		float n_angle; // angle to nearest part
		float angle_diff;
		rx=parts[pNear].x-x;
		ry=parts[pNear].y-y;
		if (rx!=0 || ry!=0)
			n_angle = atan2f(-ry, rx);
		else
			n_angle = 0;
		if (n_angle<0)
			n_angle+=M_PI*2;
		angle_diff = fabsf(n_angle-parts[i].tmp*M_PI/180);
		if (angle_diff>M_PI)
			angle_diff = M_PI*2 - angle_diff;
		if (parts[i].life<5 || angle_diff<M_PI*0.8) // lightning strike
		{
			create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, parts[i].tmp-90, 0);

			if (t!=PT_TESC)
			{
				pNear=contact_part(sim, pNear, PT_LIGH);
				if (pNear!=-1)
				{
					parts[pNear].tmp2=3;
					parts[pNear].life=(int)(1.0*parts[i].life/2-1);
					parts[pNear].tmp=parts[i].tmp-180;
					parts[pNear].temp=parts[i].temp;
				}
			}
		}
		else pNear=-1;
	}*/

	//if (parts[i].tmp2==1/* || near!=-1*/)
	//angle=0;//parts[i].tmp-30+rand()%60;
	angle = (parts[i].tmp-30+rand()%60)%360;
	multipler=parts[i].life*1.5+rand()%((int)(parts[i].life+1));	
	rx=cos(angle*M_PI/180)*multipler;
	ry=-sin(angle*M_PI/180)*multipler;
	create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, angle, parts[i].tmp2);
	if (parts[i].tmp2==2)// && pNear==-1)
	{
		angle2= ((int)angle+100-rand()%200)%360;
		rx=cos(angle2*M_PI/180)*multipler;
		ry=-sin(angle2*M_PI/180)*multipler;
		create_line_par(sim, x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, angle2, parts[i].tmp2);
	}

	parts[i].tmp2=-1;
	return 1;
}

//#TPT-Directive ElementHeader Element_LIGH static int LIGH_nearest_part(Simulation * sim, int ci, int max_d)
int Element_LIGH::LIGH_nearest_part(Simulation * sim, int ci, int max_d)
{
	int distance = (max_d!=-1)?max_d:MAX_DISTANCE;
	int ndistance = 0;
	int id = -1;
	int i = 0;
	int cx = (int)sim->parts[ci].x;
	int cy = (int)sim->parts[ci].y;
	for (i=0; i<=sim->parts_lastActiveIndex; i++)
	{
		if (sim->parts[i].type && sim->parts[i].life && i!=ci && sim->parts[i].type!=PT_LIGH && sim->parts[i].type!=PT_THDR && sim->parts[i].type!=PT_NEUT && sim->parts[i].type!=PT_PHOT)
		{
			ndistance = abs(cx-sim->parts[i].x)+abs(cy-sim->parts[i].y);// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

//#TPT-Directive ElementHeader Element_LIGH static int contact_part(Simulation * sim, int i, int tp)
int Element_LIGH::contact_part(Simulation * sim, int i, int tp)
{
	int x=sim->parts[i].x, y=sim->parts[i].y;
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = sim->pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==tp)
					return r>>8;
			}
	return -1;
}

//#TPT-Directive ElementHeader Element_LIGH static bool create_LIGH(Simulation * sim, int x, int y, int c, int temp, int life, int tmp, int tmp2, bool last)
bool Element_LIGH::create_LIGH(Simulation * sim, int x, int y, int c, int temp, int life, int tmp, int tmp2, bool last)
{
	int p = sim->create_part(-1, x, y,c);
	if (p != -1)
	{
		sim->parts[p].temp = temp;
		sim->parts[p].tmp = tmp;
		if (last)
		{
			sim->parts[p].tmp2=1+(rand()%200>tmp2*tmp2/10+60);
			sim->parts[p].life=(int)(life/1.5-rand()%2);
		}
		else
		{
			sim->parts[p].life = life;
			sim->parts[p].tmp2 = 0;
		}
	}
	else if (x >= 0 && x < XRES && y >= 0 && y < YRES)
	{
		int r = sim->pmap[y][x];
		if ((((r&0xFF)==PT_VOID || ((r&0xFF)==PT_PVOD && sim->parts[r>>8].life >= 10)) && (!sim->parts[r>>8].ctype || (sim->parts[r>>8].ctype==c)!=(sim->parts[r>>8].tmp&1))) || (r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) // VOID, PVOD, VACU, and BHOL eat LIGH here
			return true;
	}
	else return true;
	return false;
}

//#TPT-Directive ElementHeader Element_LIGH static void create_line_par(Simulation * sim, int x1, int y1, int x2, int y2, int c, int temp, int life, int tmp, int tmp2)
void Element_LIGH::create_line_par(Simulation * sim, int x1, int y1, int x2, int y2, int c, int temp, int life, int tmp, int tmp2)
{
	bool reverseXY = abs(y2-y1) > abs(x2-x1), back = false;
	int x, y, dx, dy, Ystep;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
		back = 1;
	dx = x2 - x1;
	dy = abs(y2 - y1);
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	Ystep = (y1<y2) ? 1 : -1;
	if (!back)
	{
		for (x = x1; x <= x2; x++)
		{
			bool ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2,x==x2);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2,x==x2);
			if (ret)
				return;

			e += de;
			if (e >= 0.5f)
			{
				y += Ystep;
				e -= 1.0f;
			}
		}
	}
	else
	{
		for (x = x1; x >= x2; x--)
		{
			bool ret;
			if (reverseXY)
				ret = create_LIGH(sim, y, x, c, temp, life, tmp, tmp2,x==x2);
			else
				ret = create_LIGH(sim, x, y, c, temp, life, tmp, tmp2,x==x2);
			if (ret)
				return;

			e += de;
			if (e <= -0.5f)
			{
				y += Ystep;
				e += 1.0f;
			}
		}
	}
}


//#TPT-Directive ElementHeader Element_LIGH static int graphics(GRAPHICS_FUNC_ARGS)
int Element_LIGH::graphics(GRAPHICS_FUNC_ARGS)

{
	*firea = 120;
	*firer = *colr = 235;
	*fireg = *colg = 245;
	*fireb = *colb = 255;
	*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	return 1;
}


Element_LIGH::~Element_LIGH() {}
