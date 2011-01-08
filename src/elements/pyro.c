#include <powder.h>

int update_PYRO(UPDATE_FUNC_ARGS) {
	int r, rt, lpv, t = parts[i].type;
	if (t==PT_PLSM&&parts[i].ctype == PT_NBLE&&parts[i].life <=1)
	{
		parts[i].type = PT_NBLE;
		parts[i].life = 0;
	}
	if (t==PT_FIRE && parts[i].life <=1 && parts[i].temp<625)
	{
		t = parts[i].type = PT_SMKE;
		parts[i].life = rand()%20+250;
	}
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (bmap[(y+ny)/CELL][(x+nx)/CELL] && bmap[(y+ny)/CELL][(x+nx)/CELL]!=WL_STREAM)
					continue;
				rt = parts[r>>8].type;
				if ((a || ptypes[rt].explosive) && (t!=PT_SPRK || (rt!=PT_RBDM && rt!=PT_LRBD && rt!=PT_INSL && rt!=PT_SWCH)) &&
				        !(t==PT_PHOT && rt==PT_INSL) &&
				        (t!=PT_LAVA || parts[i].life>0 || (rt!=PT_STNE && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_METL  && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SWCH && rt!=PT_INWR && rt!=PT_QRTZ))
				        && !(rt==PT_SPNG && parts[r>>8].life>0) &&
				        ptypes[rt].flammable && (ptypes[rt].flammable + (int)(pv[(y+ny)/CELL][(x+nx)/CELL]*10.0f))>(rand()%1000))
				{
					parts[r>>8].type = PT_FIRE;
					parts[r>>8].temp = ptypes[PT_FIRE].heat + (ptypes[rt].flammable/2);
					parts[r>>8].life = rand()%80+180;
					if (ptypes[rt].explosive)
						pv[y/CELL][x/CELL] += 0.25f * CFDS;
					continue;
				}
				lpv = (int)pv[(y+ny)/CELL][(x+nx)/CELL];
				if (lpv < 1) lpv = 1;
				if (legacy_enable)
				{
					if (t!=PT_SPRK && ptypes[rt].meltable  && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) &&
					        ptypes[rt].meltable*lpv>(rand()%1000))
					{
						if (t!=PT_LAVA || parts[i].life>0)
						{
							parts[r>>8].ctype = (parts[r>>8].type==PT_BRMT)?PT_BMTL:parts[r>>8].type;
							parts[r>>8].ctype = (parts[r>>8].ctype==PT_SAND)?PT_GLAS:parts[r>>8].ctype;
							parts[r>>8].type = PT_LAVA;
							parts[r>>8].life = rand()%120+240;
						}
						else
						{
							parts[i].life = 0;
							t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
							parts[i].ctype = PT_NONE;//rt;
							return 1;
						}
					}
					if (t!=PT_SPRK && (rt==PT_ICEI || rt==PT_SNOW))
					{
						parts[r>>8].type = PT_WATR;
						if (t==PT_FIRE)
						{
							parts[i].x = lx;
							parts[i].y = ly;
							kill_part(i);
							return 1;
						}
						if (t==PT_LAVA)
						{
							parts[i].life = 0;
							t = parts[i].type = PT_STNE;
							return 1;
						}
					}
					if (t!=PT_SPRK && (rt==PT_WATR || rt==PT_DSTW || rt==PT_SLTW))
					{
						kill_part(r>>8);
						if (t==PT_FIRE)
						{
							parts[i].x = lx;
							parts[i].y = ly;
							kill_part(i);
							return 1;
						}
						if (t==PT_LAVA)
						{
							parts[i].life = 0;
							t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
							parts[i].ctype = PT_NONE;
							return 1;
						}
					}
				}
			}
	return 0;
}
