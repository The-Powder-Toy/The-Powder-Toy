#include <powder.h>

int update_THRM(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM || (r&0xFF)==PT_LAVA))
				{
					if (1>(rand()%500)) {
						parts[i].type = PT_LAVA;
						parts[i].ctype = PT_BMTL;
						parts[i].temp = 3500.0f;
						pv[y/CELL][x/CELL] += 50.0f;
					} else {
						parts[i].type = PT_LAVA;
						parts[i].life = 400;
						parts[i].ctype = PT_THRM;
						parts[i].temp = 3500.0f;
						parts[i].tmp = 20;
					}
				}
				//if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
				//t = parts[i].type = PT_WATR;
			}
	return 0;
}
