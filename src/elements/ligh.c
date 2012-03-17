#include <element.h>

#define LIGHTING_POWER 0.65

int LIGH_nearest_part(int ci, int max_d)
{
	int distance = (max_d!=-1)?max_d:MAX_DISTANCE;
	int ndistance = 0;
	int id = -1;
	int i = 0;
	int cx = (int)parts[ci].x;
	int cy = (int)parts[ci].y;
	for (i=0; i<=parts_lastActiveIndex; i++)
	{
		if (parts[i].type && !parts[i].life && i!=ci && parts[i].type!=PT_LIGH && parts[i].type!=PT_THDR && parts[i].type!=PT_NEUT && parts[i].type!=PT_PHOT)
		{
			ndistance = abs(cx-parts[i].x)+abs(cy-parts[i].y);// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

int contact_part(int i, int tp)
{
	int x=parts[i].x, y=parts[i].y;
	int r,rx,ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==tp)
					return r>>8;
			}
	return -1;
}

void create_line_par(int x1, int y1, int x2, int y2, int c, int temp, int life, int tmp, int tmp2)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
	float e, de;
	if (c==WL_EHOLE || c==WL_ALLOWGAS || c==WL_ALLOWALLELEC || c==WL_ALLOWSOLID || c==WL_ALLOWAIR || c==WL_WALL || c==WL_DESTROYALL || c==WL_ALLOWLIQUID || c==WL_FAN || c==WL_STREAM || c==WL_DETECT || c==WL_EWALL || c==WL_WALLELEC)
		return; // this function only for particles, no walls
	if (cp)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	e = 0.0f;
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		int p;
		if (cp)
			p=create_part(-1, y, x, c);
		else
			p=create_part(-1, x, y,c);
		if (p!=-1)
		{
			parts[p].life=life;
			parts[p].temp=temp;
			parts[p].tmp=tmp;
			parts[p].tmp2=tmp2;
		}
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			e -= 1.0f;
		}
	}
}

int update_LIGH(UPDATE_FUNC_ARGS)
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
	 * tmp - angle of lighting
	 *
	*/
	int r,rx,ry, multipler, powderful;
	float angle, angle2=-1;
	int near;
	powderful = powderful=parts[i].temp*(1+parts[i].life/40)*LIGHTING_POWER;
	update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
	if (aheat_enable)
	{
		hv[y/CELL][x/CELL]+=powderful/50;
		if (hv[y/CELL][x/CELL]>MAX_TEMP)
			hv[y/CELL][x/CELL]=MAX_TEMP;
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_LIGH && (r&0xFF)!=PT_TESC)
				{
					if ((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT)
					{
						if ((ptypes[r&0xFF].properties&PROP_CONDUCTS) && parts[r>>8].life==0)
						{
							create_part(r>>8,x+rx,y+ry,PT_SPRK);
						}
						pv[y/CELL][x/CELL] += powderful/400;
						if (ptypes[r&0xFF].hconduct) parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/1.5, MIN_TEMP, MAX_TEMP);
					}
					if ((r&0xFF)==PT_DEUT || (r&0xFF)==PT_PLUT) // start nuclear reactions
					{
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful, MIN_TEMP, MAX_TEMP);
						pv[y/CELL][x/CELL] +=powderful/35;
						if (rand()%3==0)
						{
							part_change_type(r>>8,x+rx,y+ry,PT_NEUT);
							parts[r>>8].life = rand()%480+480;
							parts[r>>8].vx=rand()%10-5;
							parts[r>>8].vy=rand()%10-5;
						}
					}
					if ((r&0xFF)==PT_COAL || (r&0xFF)==PT_BCOL) // ignite coal
					{
						if (parts[r>>8].life>100) {
							parts[r>>8].life = 99;
						}
					}
					if (ptypes[r&0xFF].hconduct)
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/10, MIN_TEMP, MAX_TEMP);
					if (((r&0xFF)==PT_STKM && player.elem!=PT_LIGH) || ((r&0xFF)==PT_STKM2 && player2.elem!=PT_LIGH))
					{
						parts[r>>8].life-=powderful/100;
					}
				}
			}
	if (parts[i].tmp2==3)
	{
		parts[i].tmp2=0;
		return 1;
	}

	if (parts[i].tmp2==-1)
	{
		kill_part(i);
		return 1;
	}
	if (parts[i].tmp2<=0 || parts[i].life<=1)
	{
		if (parts[i].tmp2>0)
			parts[i].tmp2=0;
		parts[i].tmp2--;
		return 1;
	}
	if (parts[i].tmp2<=-2)
	{
		kill_part(i);
		return 1;
	}

	angle2=-1;

	near = LIGH_nearest_part(i, parts[i].life*2.5);
	if (near!=-1)
	{
		int t=parts[near].type;
		float n_angle; // angle to nearest part
		rx=parts[near].x-x;
		ry=parts[near].y-y;
		if (rx*rx+ry*ry!=0)
			n_angle = asin(-ry/sqrt(rx*rx+ry*ry));
		else
			n_angle = 0;
		if (n_angle<0)
			n_angle+=M_PI*2;
		if (parts[i].life<5 || fabs(n_angle-parts[i].tmp*M_PI/180)<M_PI*0.8) // lightning strike
		{
			create_line_par(x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, parts[i].tmp-90, 0);

			if (t!=PT_TESC)
			{
				near=contact_part(near, PT_LIGH);
				if (near!=-1)
				{
					parts[near].tmp2=3;
					parts[near].life=(int)(1.0*parts[i].life/2-1);
					parts[near].tmp=parts[i].tmp-180;
					parts[near].temp=parts[i].temp;
				}
			}
		}
		else near=-1;
	}

	//if (parts[i].tmp2==1/* || near!=-1*/)
	//angle=0;//parts[i].tmp-30+rand()%60;
	angle = parts[i].tmp-30+rand()%60;
	if (angle<0)
		angle+=360;
	if (angle>=360)
		angle-=360;
	if (parts[i].tmp2==2 && near==-1)
	{
		angle2=angle+100-rand()%200;
		if (angle2<0)
			angle2+=360;
		if (angle2>=360)
			angle-=360;
	}

	multipler=parts[i].life*1.5+rand()%((int)(parts[i].life+1));
	rx=cos(angle*M_PI/180)*multipler;
	ry=-sin(angle*M_PI/180)*multipler;
	create_line_par(x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, angle, 0);

	if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if ((r&0xFF)==PT_LIGH)
		{
			parts[r>>8].tmp2=1+(rand()%200>parts[i].tmp2*parts[i].tmp2/10+60);
			parts[r>>8].life=(int)(1.0*parts[i].life/1.5-rand()%2);
			parts[r>>8].tmp=angle;
			parts[r>>8].temp=parts[i].temp;
		}
	}

	if (angle2!=-1)
	{
		multipler=parts[i].life*1.5+rand()%((int)(parts[i].life+1));
		rx=cos(angle2*M_PI/180)*multipler;
		ry=-sin(angle2*M_PI/180)*multipler;
		create_line_par(x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, angle2, 0);

		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r&0xFF)==PT_LIGH)
			{
				parts[r>>8].tmp2=1+(rand()%200>parts[i].tmp2*parts[i].tmp2/10+40);
				parts[r>>8].life=(int)(1.0*parts[i].life/1.5-rand()%2);
				parts[r>>8].tmp=angle;
				parts[r>>8].temp=parts[i].temp;
			}
		}
	}

	parts[i].tmp2=-1;
	return 1;
}
int graphics_LIGH(GRAPHICS_FUNC_ARGS)
{
	*firea = 120;
	*firer = *colr = 235;
	*fireg = *colg = 245;
	*fireb = *colb = 255;
	*pixel_mode |= PMODE_GLOW | FIRE_ADD;
	return 1;
}
