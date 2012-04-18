#include "simulation/Element.h"

int update_PYRO(UPDATE_FUNC_ARGS) {
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
				if ((surround_space || sim->ptypes[rt].explosive) &&
					(t!=PT_SPRK || (rt!=PT_RBDM && rt!=PT_LRBD && rt!=PT_INSL)) &&
					(t!=PT_PHOT || rt!=PT_INSL) &&
				    (rt!=PT_SPNG || parts[r>>8].life==0) &&
				    (rt!=PT_H2 || (parts[r>>8].temp < 2273.15 && sim->pv[y/CELL][x/CELL] < 50.0f)) &&
				    sim->ptypes[rt].flammable && (sim->ptypes[rt].flammable + (int)(sim->pv[(y+ry)/CELL][(x+rx)/CELL]*10.0f))>(rand()%1000))
				{
					sim->part_change_type(r>>8,x+rx,y+ry,PT_FIRE);
					parts[r>>8].temp = restrict_flt(sim->ptypes[PT_FIRE].heat + (sim->ptypes[rt].flammable/2), MIN_TEMP, MAX_TEMP);
					parts[r>>8].life = rand()%80+180;
					parts[r>>8].tmp = parts[r>>8].ctype = 0;
					if (sim->ptypes[rt].explosive)
						sim->pv[y/CELL][x/CELL] += 0.25f * CFDS;
				}
			}
	if (sim->legacy_enable) update_legacy_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

int update_legacy_PYRO(UPDATE_FUNC_ARGS) {
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
				if (t!=PT_SPRK && sim->ptypes[rt].meltable  && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) &&
						sim->ptypes[rt].meltable*lpv>(rand()%1000))
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
