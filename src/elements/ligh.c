#include <element.h>

#define LIGHTING_POWER 0.65

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
	if (c==WL_EHOLE || c==WL_ALLOWGAS || c==WL_ALLOWALLELEC || c==WL_ALLOWSOLID || c==WL_ALLOWAIR || c==WL_WALL || c==WL_DESTROYALL || c==WL_ALLOWLIQUID || c==WL_FAN || c==WL_STREAM || c==WL_DETECT || c==WL_EWALL || c==WL_WALLELEC)
		return; // this function only for particles, no walls
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
	float e, de;
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
	int r,rx,ry, multipler, powderful=parts[i].temp*(1+parts[i].life/40)*LIGHTING_POWER;
	hv[y/CELL][x/CELL]+=powderful/40;
	if (hv[y/CELL][x/CELL]>MAX_TEMP)
		hv[y/CELL][x/CELL]=MAX_TEMP;
	for (rx=-2; rx<3; rx++) // destruction
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_LIGH && (r&0xFF)!=PT_TESC)
				{
					if (parts[i].tmp2==3)
					{
						if ((ptypes[r&0xFF].properties&PROP_CONDUCTS) && parts[r>>8].life==0)
						{
							parts[r>>8].ctype = parts[r>>8].type;
							create_part(r>>8,x+rx,y+ry,PT_SPRK);
							parts[r>>8].life = 4;

							pv[y/CELL][x/CELL] += powderful/1200;
							parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/20, MIN_TEMP, MAX_TEMP);
						}
						else if ((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_SPRK&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT)
						{
							pv[y/CELL][x/CELL] += powderful/110;
							parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/1.5, MIN_TEMP, MAX_TEMP);
						}
						if ((r&0xFF)==PT_DEUT || (r&0xFF)==PT_PLUT)
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
					}
					if ((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_SPRK&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT)
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/16, MIN_TEMP, MAX_TEMP);
					else
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp+powderful/70, MIN_TEMP, MAX_TEMP);
				}
			}
	if (parts[i].tmp2==3)
	{
		//if (rand()&1)
		parts[i].tmp2=1;
		/*else
			parts[i].tmp2=0;*/
		return 1;
	}

	if (parts[i].tmp2==-1)
	{
		kill_part(i);
		return 1;
	}
	if (parts[i].tmp2<=0 || parts[i].life<=1)
	{
		/*if (parts[i].tmp2!=3 && rand()%3000<400/(parts[i].life+1))
		{
			parts[i].tmp2=1;
			return 1;
		}*/
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

	float angle, angle2=-1;

	int near=nearest_part(i, -1, parts[i].life*3.5);
	if (near!=-1 && (parts[near].type==PT_LIGH || parts[near].type==PT_THDR || parts[near].type==PT_NEUT))
		near=-1;
	if (near!=-1)
	{
		int t=parts[near].type;

		rx=parts[near].x-x;
		ry=parts[near].y-y;
		if (asin(ry-sqrt(rx*rx+ry*ry))<M_PI*2/3)
		{
			create_line_par(x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, parts[i].tmp-90, 0);

			/*if (t!=PT_DMND && t!=PT_UDMT && t!=PT_TESC && parts[i].temp-300>rand()%2000)
			{
				part_change_type(near, x+rx, y+ry, PT_LIGH);
				parts[near].tmp2=3;
				parts[near].life=(int)(1.0*parts[i].life/2-1);
				parts[near].tmp=parts[i].tmp-180;
				parts[near].temp=parts[i].temp;
			}*/
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
	switch (gravityMode)
	{
	default:
	case 0:
		angle = parts[i].tmp-30+rand()%60;
		break;
	case 1:
		angle = rand()%360;
		break;
	case 2:
		angle = atan2(x-XCNTR, y-YCNTR)*(180.0f/M_PI)+90;
	}
	//angle=0;//parts[i].tmp-30+rand()%60;
	if (angle<0)
		angle+=360;
	if (angle>=360)
		angle-=360;
	if (parts[i].tmp2==2)
	{
		angle2=angle+100-rand()%200;
		if (angle2<0)
			angle2+=360;
		if (angle2>=360)
			angle-=360;
		/*angle=parts[i].tmp-70+rand()%50;
		if (angle<0)
			angle+=360;
		if (angle>=360)
			angle-=360;*/
	}

	multipler=parts[i].life*1.5+rand()%((int)(parts[i].life+1));
	rx=cos(angle*M_PI/180)*multipler;
	ry=-sin(angle*M_PI/180)*multipler;
	create_line_par(x, y, x+rx, y+ry, PT_LIGH, parts[i].temp, parts[i].life, angle, 0);

	if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		/*if ((r&0xFF)!=PT_LIGH && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_UDMT && (r&0xFF)!=PT_TESC)
				part_change_type(r>>8, x+rx, y+ry, PT_LIGH);*/
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
			/*if ((r&0xFF)!=PT_LIGH && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_UDMT && (r&0xFF)!=PT_TESC)
				part_change_type(r>>8, x+rx, y+ry, PT_LIGH);*/
			if ((r&0xFF)==PT_LIGH && !((r>>8)>=NPART || !r))
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
