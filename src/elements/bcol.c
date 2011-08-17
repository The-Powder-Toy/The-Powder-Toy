#include <element.h>

int update_BCOL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life<=0) {
		create_part(i, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
	}

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
				{
					if (parts[i].life>100) {
						parts[i].life = 99;
					}
				}
				if ((r&0xFF)==PT_LAVA && 1>(rand()%500))
				{
					if (parts[r>>8].ctype == PT_IRON) {
						parts[r>>8].ctype = PT_METL;
						kill_part(i);
                                                return 1;
					}
				}
			}
	return 0;
}
