#include <element.h>

int update_SPRK(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt, conduct_sprk, nearp, pavg, ct = parts[i].ctype;
	update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);

	if (parts[i].life<=0)
	{
		if (ct==PT_WATR||ct==PT_COPR||ct==PT_SLTW||ct==PT_PSCN||ct==PT_NSCN||ct==PT_ETRD||ct==PT_INWR)
			parts[i].temp = R_TEMP + 273.15f;
		if (!ct)
			ct = PT_METL;
		part_change_type(i,x,y,ct);
		parts[i].ctype = PT_NONE;
		parts[i].life = 4;
		if (ct == PT_WATR)
			parts[i].life = 64;
        if (ct == PT_COPR)
			parts[i].life = 64;
		if (ct == PT_SLTW)
			parts[i].life = 54;
		if (ct == PT_SWCH)
			parts[i].life = 14;
        if (ct == PT_PIVS)
			parts[i].life = 14;
		return 0;
	}
	if (ct==PT_SPRK)
	{
		kill_part(i);
		return 1;
	}
	else if (ct==PT_NTCT || ct==PT_PTCT)
	{
		update_NPTCT(UPDATE_FUNC_SUBCALL_ARGS);
	}
    else if (ct==PT_OSMT)
	{
		parts[i].ctype = PT_IFIL;
	}
	else if (ct==PT_ETRD&&parts[i].life==1)
	{
		nearp = nearest_part(i, PT_ETRD);
		if (nearp!=-1&&parts_avg(i, nearp, PT_INSL)!=PT_INSL)
		{
			create_line(x, y, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), 0, 0, PT_PLSM);
			part_change_type(i,x,y,ct);
			ct = parts[i].ctype = PT_NONE;
			parts[i].life = 20;
			part_change_type(nearp,(int)(parts[nearp].x+0.5f),(int)(parts[nearp].y+0.5f),PT_SPRK);
			parts[nearp].life = 9;
			parts[nearp].ctype = PT_ETRD;
		}
	}
	else if (ct==PT_CTRD&&parts[i].life==1)
	{
		nearp = nearest_part(i, PT_CTRD);
		if (nearp!=-1&&parts_avg(i, nearp, PT_INSL)!=PT_INSL)
		{
			create_line(x, y, (int)(parts[nearp].x+0.5f), (int)(parts[nearp].y+0.5f), 0, 0, PT_HFLM);
			part_change_type(i,x,y,ct);
			ct = parts[i].ctype = PT_NONE;
			parts[i].life = 20;
			part_change_type(nearp,(int)(parts[nearp].x+0.5f),(int)(parts[nearp].y+0.5f),PT_SPRK);
			parts[nearp].life = 9;
			parts[nearp].ctype = PT_CTRD;
		}
	}
	else if (ct==PT_NBLE&&parts[i].life<=1)
	{
		parts[i].life = rand()%150+50;
		part_change_type(i,x,y,PT_PLSM);
		parts[i].ctype = PT_NBLE;
		parts[i].temp = 3500;
		pv[y/CELL][x/CELL] += 1;
		parts[i].r = 235;
        parts[i].g = 73;
        parts[i].b = 23;
        parts[i].temp = 3500;
        pv[y/CELL][x/CELL] += 1;
    }
    else if (ct==PT_ARGN&&parts[i].life<=1)
    {
        parts[i].life = rand()%150+50;
        part_change_type(i,x,y,PT_PLSM);
        parts[i].ctype = PT_ARGN;
        parts[i].r = 227;
        parts[i].g = 73;
        parts[i].b = 206;
	}
	else if (ct==PT_IRON) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
					if (((r&TYPE) == PT_DSTW && 30>(rand()/(RAND_MAX/1000))) ||
					        ((r&TYPE) == PT_SLTW && 30>(rand()/(RAND_MAX/1000))) ||
					        ((r&TYPE) == PT_WATR && 30>(rand()/(RAND_MAX/1000))))
					{
						if (rand()<RAND_MAX/3)
                            part_change_type(r>>PS,x+rx,y+ry,PT_O2);
                        else
                            part_change_type(r>>PS,x+rx,y+ry,PT_H2);
					}
				}
	}
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				rt = (r&TYPE);
				conduct_sprk = 1;


				pavg = parts_avg(r>>PS, i,PT_INSL);
				if ((rt==PT_SWCH||(rt==PT_SPRK&&parts[r>>PS].ctype==PT_SWCH)) && pavg!=PT_INSL) // make sparked SWCH turn off correctly
				{
					if (rt==PT_SWCH&&ct==PT_PSCN&&parts[r>>PS].life<10) {
						parts[r>>PS].life = 10;
					}
					if (ct==PT_NSCN) {
						part_change_type(r>>PS,x+rx,y+ry,PT_SWCH);
						parts[r>>PS].ctype = PT_NONE;
						parts[r>>PS].life = 9;
					}
				}
				else if ((ct==PT_PSCN||ct==PT_NSCN) && (rt==PT_PUMP||rt==PT_HSWC||(rt==PT_LCRY&&abs(rx)<2&&abs(ry)<2)))
				{
					if (ct==PT_PSCN) parts[r>>PS].life = 10;
					else if (ct==PT_NSCN && parts[r>>PS].life>=10) parts[r>>PS].life = 9;
				}
                else if ((ct==PT_PSCN||ct==PT_NSCN) && (rt==PT_PIVS))
				{
					if (ct==PT_PSCN) parts[r>>PS].life = 10;
					else if (ct==PT_NSCN && parts[r>>PS].life>=10) parts[r>>PS].life = 9;
				}


				// ct = spark from material, rt = spark to material. Make conduct_sprk = 0 if conduction not allowed

				if (pavg == PT_INSL) conduct_sprk = 0;
				if (!(ptypes[rt].properties&PROP_CONDUCTS||rt==PT_INST||rt==PT_QRTZ)) conduct_sprk = 0;
				if (abs(rx)+abs(ry)>=4 &&ct!=PT_SWCH&&rt!=PT_SWCH)
					conduct_sprk = 0;


				if (ct==PT_METL && (rt==PT_NTCT||rt==PT_PTCT||rt==PT_NCGN||rt==PT_INWR||(rt==PT_SPRK&&(parts[r>>PS].ctype==PT_NTCT||parts[r>>PS].ctype==PT_PTCT))) && pavg!=PT_INSL)//&& pavg!=PT_INSL
				{
					parts[r>>PS].temp = 473.0f;
					if (rt==PT_NTCT||rt==PT_PTCT)
						conduct_sprk = 0;
				}
				if (ct==PT_NTCT && !(rt==PT_PSCN || rt==PT_NTCT || (rt==PT_NSCN&&parts[i].temp>373.0f)))
					conduct_sprk = 0;
				if (ct==PT_PTCT && !(rt==PT_PSCN || rt==PT_PTCT || (rt==PT_NSCN&&parts[i].temp<373.0f)))
					conduct_sprk = 0;
				if (ct==PT_INWR && !(rt==PT_NSCN || rt==PT_INWR || rt==PT_PSCN))
					conduct_sprk = 0;
				if (ct==PT_NSCN && rt==PT_PSCN)
					conduct_sprk = 0;
				if (ct==PT_ETRD && !(rt==PT_METL||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN))
					conduct_sprk = 0;
				if (ct==PT_INST&&rt!=PT_NSCN) conduct_sprk = 0;
				if (ct==PT_SWCH && (rt==PT_PSCN||rt==PT_NSCN||rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR))
					conduct_sprk = 0;
				if (rt==PT_QRTZ && !((ct==PT_NSCN||ct==PT_METL||ct==PT_PSCN||ct==PT_QRTZ) && (parts[r>>PS].temp<173.15||pv[(y+ry)/CELL][(x+rx)/CELL]>8)))
					conduct_sprk = 0;
				if (rt==PT_NTCT && !(ct==PT_NSCN || ct==PT_NTCT || (ct==PT_PSCN&&parts[r>>PS].temp>373.0f)))
					conduct_sprk = 0;
				if (rt==PT_PTCT && !(ct==PT_NSCN || ct==PT_PTCT || (ct==PT_PSCN&&parts[r>>PS].temp<373.0f)))
					conduct_sprk = 0;
				if (rt==PT_INWR && !(ct==PT_NSCN || ct==PT_INWR || ct==PT_PSCN))
					conduct_sprk = 0;
				if (rt==PT_INST&&ct!=PT_PSCN)
					conduct_sprk = 0;

				if (conduct_sprk) {
					if (rt==PT_WATR||rt==PT_SLTW) {
						if (parts[r>>PS].life==0 && (parts[i].life<2 || ((r>>PS)<i && parts[i].life<3)))
						{
							part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
							if (rt==PT_WATR) parts[r>>PS].life = 6;
							else parts[r>>PS].life = 5;
							parts[r>>PS].ctype = rt;
						}
					}
					else if (rt==PT_INST) {
						if (parts[i].life>=3&&parts[r>>PS].life==0)
						{
							flood_parts(x+rx,y+ry,PT_SPRK,PT_INST,-1);//spark the wire
						}
					}
					else if (parts[r>>PS].life==0 && (parts[i].life<3 || ((r>>PS)<i && parts[i].life<4))) {
						parts[r>>PS].life = 4;
						parts[r>>PS].ctype = rt;
						part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
						if (parts[r>>PS].temp+10.0f<673.0f&&!legacy_enable&&(rt==PT_METL||rt==PT_BMTL||rt==PT_BRMT||rt==PT_PSCN||rt==PT_NSCN||rt==PT_ETRD||rt==PT_NBLE||rt==PT_IRON))
							parts[r>>PS].temp = parts[r>>PS].temp+10.0f;
					}
					else if (ct==PT_ETRD && parts[i].life==5)
					{
						part_change_type(i,x,y,ct);
						parts[i].ctype = PT_NONE;
						parts[i].life = 20;
						parts[r>>PS].life = 4;
						parts[r>>PS].ctype = rt;
						part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
					}
				}
			}
	return 0;
}
