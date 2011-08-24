#include <element.h>

int update_THRM(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM || (r&0xFF)==PT_LAVA)) // TODO: could this go in update_PYRO?
				{
					if (1>(rand()%500)) {
						part_change_type(i,x,y,PT_LAVA);
						parts[i].ctype = PT_BMTL;
						parts[i].temp = 3500.0f;
						pv[y/CELL][x/CELL] += 50.0f;
					} else {
						part_change_type(i,x,y,PT_LAVA);
						parts[i].life = 400;
						parts[i].ctype = PT_THRM;
						parts[i].temp = 3500.0f;
						parts[i].tmp = 20;
					}
				}
			}
	return 0;
}
