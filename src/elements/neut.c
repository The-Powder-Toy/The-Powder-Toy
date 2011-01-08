#include <powder.h>

int update_NEUT(UPDATE_FUNC_ARGS) {
	int r;
	int rt = 3 + (int)pv[y/CELL][x/CELL];
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW)
				{
					parts[i].vx *= 0.995;
					parts[i].vy *= 0.995;
				}
				if ((r&0xFF)==PT_PLUT && rt>(rand()%1000))
				{
					if (33>rand()%100)
					{
						create_part(r>>8, x+nx, y+ny, rand()%3 ? PT_LAVA : PT_URAN);
						parts[r>>8].temp = MAX_TEMP;
						if (parts[r>>8].type==PT_LAVA) {
							parts[r>>8].tmp = 100;
							parts[r>>8].ctype = PT_PLUT;
						}
					}
					else
					{
						create_part(r>>8, x+nx, y+ny, PT_NEUT);
						parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
						parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
					}
					pv[y/CELL][x/CELL] += 10.0f * CFDS; //Used to be 2, some people said nukes weren't powerful enough
					update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
				}
				if ((r&0xFF)==PT_DEUT && (rt+1)>(rand()%1000))
				{
#ifdef SDEUT
					create_n_parts(parts[r>>8].life, x+nx, y+ny, parts[i].vx, parts[i].vy, PT_NEUT);
#else
					create_part(r>>8, x+nx, y+ny, PT_NEUT);
					parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
					parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
					if (parts[r>>8].life>0)
					{
						parts[r>>8].life --;
						parts[r>>8].temp += (parts[r>>8].life*17);
						pv[y/CELL][x/CELL] += 6.0f * CFDS;

					}
					else
						parts[r>>8].type = PT_NONE;
#endif
				}
				if ((r&0xFF)==PT_GUNP && 15>(rand()%1000))
					parts[r>>8].type = PT_DUST;
				if ((r&0xFF)==PT_DYST && 15>(rand()%1000))
					parts[r>>8].type = PT_YEST;
				if ((r&0xFF)==PT_YEST) {
					if (15>(rand()%100000)&&isplayer==0)
						parts[r>>8].type = PT_STKM;
					else
						parts[r>>8].type = PT_DYST;
				}

				if ((r&0xFF)==PT_WATR && 15>(rand()%100))
					parts[r>>8].type = PT_DSTW;
				if ((r&0xFF)==PT_PLEX && 15>(rand()%1000))
					parts[r>>8].type = PT_GOO;
				if ((r&0xFF)==PT_NITR && 15>(rand()%1000))
					parts[r>>8].type = PT_DESL;
				if ((r&0xFF)==PT_PLNT && 5>(rand()%100))
					parts[r>>8].type = PT_WOOD;
				if ((r&0xFF)==PT_DESL && 15>(rand()%1000))
					parts[r>>8].type = PT_GAS;
				if ((r&0xFF)==PT_COAL && 5>(rand()%100))
					parts[r>>8].type = PT_WOOD;
				if ((r&0xFF)==PT_DUST && 5>(rand()%100))
					parts[r>>8].type = PT_FWRK;
				if ((r&0xFF)==PT_FWRK && 5>(rand()%100))
					parts[r>>8].ctype = PT_DUST;
				if ((r&0xFF)==PT_ACID && 5>(rand()%100))
				{
					parts[r>>8].type = PT_ISOZ;
					parts[r>>8].life = 0;
				}
				/*if(parts[r>>8].type>1 && parts[r>>8].type!=PT_NEUT && parts[r>>8].type-1!=PT_NEUT && parts[r>>8].type-1!=PT_STKM &&
				  (ptypes[parts[r>>8].type-1].menusection==SC_LIQUID||
				  ptypes[parts[r>>8].type-1].menusection==SC_EXPLOSIVE||
				  ptypes[parts[r>>8].type-1].menusection==SC_GAS||
				  ptypes[parts[r>>8].type-1].menusection==SC_POWDERS) && 15>(rand()%1000))
				  parts[r>>8].type--;*/
			}
	return 0;
}
