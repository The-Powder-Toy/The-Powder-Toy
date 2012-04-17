#include "simulation/Element.h"

int update_WATR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_SALT && 1>(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_SLTW);
					sim->part_change_type(r>>8,x+rx,y+ry,PT_SLTW);
				}
				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (sim->legacy_enable||parts[i].temp>(273.15f+12.0f)) && 1>(rand()%500))
				{
					sim->part_change_type(i,x,y,PT_FIRE);
					parts[i].life = 4;
					parts[i].ctype = PT_WATR;
				}
				if ((r&0xFF)==PT_FIRE && parts[r>>8].ctype!=PT_WATR){
					sim->kill_part(r>>8);
						if(1>(rand()%150)){
							sim->kill_part(i);
							return 1;
						}
				}
				/*if ((r&0xFF)==PT_CNCT && 1>(rand()%500))	Concrete+Water to paste, not very popular
				{
					part_change_type(i,x,y,PT_PSTE);
					sim.kill_part(r>>8);
				}*/
			}
	return 0;
}
