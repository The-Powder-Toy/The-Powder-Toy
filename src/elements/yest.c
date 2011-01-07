#include <powder.h>

int update_YEST(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_DYST && 1>(rand()%30) && !legacy_enable)
				{
					parts[i].type = PT_DYST;
				}
			}
	if (parts[i].temp>303&&parts[i].temp<317) {
		create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_YEST);
	}
	return 0;
}
