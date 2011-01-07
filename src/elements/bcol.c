#include <powder.h>

int update_BCOL(UPDATE_FUNC_ARGS) {
	int r;
	if (parts[i].life<=0) {
		parts[i].type = PT_NONE;
		kill_part(i);
		create_part(-1, x, y, PT_FIRE);
		return 1;
	} else if (parts[i].life < 100) {
		parts[i].life--;
		create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
	}

	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
				{
					if (parts[i].life>100) {
						parts[i].life = 99;
					}
				}
			}
	return 0;
}
