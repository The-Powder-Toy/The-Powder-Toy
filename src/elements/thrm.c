#include <element.h>

int update_THRM(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int self = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((parts[r>>PS].type==PT_FIRE || parts[r>>PS].type==PT_PLSM || parts[r>>PS].type==PT_LAVA)) // TODO: could this go in update_PYRO?
				{
					if (1>(rand()%500)) {
						part_change_type(i,x,y,PT_LAVA);
						parts[i].ctype = PT_BMTL;
						parts[i].temp = 3500.0f;
						pv[y/CELL][x/CELL] += 50.0f;
					} else {
						part_change_type(i,x,y,PT_LAVA);
						parts[i].life = 400;
						parts[i].ctype = self;
						parts[i].temp = 3500.0f;
						parts[i].tmp = 20;
					}
					if (1>(rand()%500)) {
						part_change_type(i,x,y,PT_MGMA);
						parts[i].ctype = PT_MGMA;
						parts[i].temp = 99999.0f;
						pv[y/CELL][x/CELL] += 10.0f;
					}
				}
			}
	return 0;
}
