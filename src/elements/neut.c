#include <element.h>

int update_NEUT(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	int pressureFactor = 3 + (int)pv[y/CELL][x/CELL];
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW)
				{
					parts[i].vx *= 0.995;
					parts[i].vy *= 0.995;
				}
				if ((r&0xFF)==PT_PLUT && pressureFactor>(rand()%1000))
				{
					if (33>rand()%100)
					{
						create_part(r>>8, x+rx, y+ry, rand()%3 ? PT_LAVA : PT_URAN);
						parts[r>>8].temp = MAX_TEMP;
						if (parts[r>>8].type==PT_LAVA) {
							parts[r>>8].tmp = 100;
							parts[r>>8].ctype = PT_PLUT;
						}
					}
					else
					{
						create_part(r>>8, x+rx, y+ry, PT_NEUT);
						parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
						parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
					}
					pv[y/CELL][x/CELL] += 10.0f * CFDS; //Used to be 2, some people said nukes weren't powerful enough
					update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
				}
				else if ((r&0xFF)==PT_DEUT && (pressureFactor+1)>(rand()%1000))
				{
#ifdef SDEUT
					create_n_parts(parts[r>>8].life, x+rx, y+ry, parts[i].vx, parts[i].vy, PT_NEUT);
#else
					create_part(r>>8, x+rx, y+ry, PT_NEUT);
					parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
					parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
					if (parts[r>>8].life>0)
					{
						parts[r>>8].life --;
						parts[r>>8].temp = restrict_flt(parts[r>>8].temp + parts[r>>8].life*17, MIN_TEMP, MAX_TEMP);
						pv[y/CELL][x/CELL] += 6.0f * CFDS;
					}
					else
						kill_part(r>>8);
#endif
				}
				else if ((r&0xFF)==PT_GUNP && 15>(rand()%1000))
					part_change_type(r>>8,x+rx,y+ry,PT_DUST);
				else if ((r&0xFF)==PT_DYST && 15>(rand()%1000))
					part_change_type(r>>8,x+rx,y+ry,PT_YEST);
				else if ((r&0xFF)==PT_YEST)
					part_change_type(r>>8,x+rx,y+ry,PT_DYST);
				else if ((r&0xFF)==PT_WATR && 15>(rand()%100))
					part_change_type(r>>8,x+rx,y+ry,PT_DSTW);
				else if ((r&0xFF)==PT_PLEX && 15>(rand()%1000))
					part_change_type(r>>8,x+rx,y+ry,PT_GOO);
				else if ((r&0xFF)==PT_NITR && 15>(rand()%1000))
					part_change_type(r>>8,x+rx,y+ry,PT_DESL);
				else if ((r&0xFF)==PT_PLNT && 5>(rand()%100))
					create_part(r>>8, x+rx, y+ry, PT_WOOD);
				else if ((r&0xFF)==PT_DESL && 15>(rand()%1000))
					part_change_type(r>>8,x+rx,y+ry,PT_GAS);
				else if ((r&0xFF)==PT_COAL && 5>(rand()%100))
					create_part(r>>8, x+rx, y+ry, PT_WOOD);
				else if ((r&0xFF)==PT_DUST && 5>(rand()%100))
					part_change_type(r>>8, x+rx, y+ry, PT_FWRK);
				else if ((r&0xFF)==PT_FWRK && 5>(rand()%100))
					parts[r>>8].ctype = PT_DUST;
				else if ((r&0xFF)==PT_ACID && 5>(rand()%100))
					create_part(r>>8, x+rx, y+ry, PT_ISOZ);
				/*if(parts[r>>8].type>1 && parts[r>>8].type!=PT_NEUT && parts[r>>8].type-1!=PT_NEUT && parts[r>>8].type-1!=PT_STKM &&
				  (ptypes[parts[r>>8].type-1].menusection==SC_LIQUID||
				  ptypes[parts[r>>8].type-1].menusection==SC_EXPLOSIVE||
				  ptypes[parts[r>>8].type-1].menusection==SC_GAS||
				  ptypes[parts[r>>8].type-1].menusection==SC_POWDERS) && 15>(rand()%1000))
				  parts[r>>8].type--;*/
			}
	r = pmap[y][x];
	rt = r&0xFF;
	if (rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN) {
		if (!parts[r>>8].ctype)
			parts[r>>8].ctype = PT_PHOT;
	}
	return 0;
}
