#include "simulation/Element.h"

int update_CO2(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
                if (20>(rand()%40000)&&parts[i].ctype==5)
				{
					parts[i].ctype = 0;
					sim->create_part(-3, x, y, PT_WATR);
				}
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_FIRE){
					sim->kill_part(r>>8);
						if(1>(rand()%150)){
							sim->kill_part(i);
							return 1;
						}
				}
				if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%250))
				{
					sim->part_change_type(i,x,y,PT_CBNW);
					sim->kill_part(r>>8);
				}
			}
	return 0;
}
