#include <powder.h>

int update_GLOW(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_WATR&&5>(rand()%2000))
				{
					parts[i].type = PT_NONE;
					parts[r>>8].type = PT_DEUT;
					parts[r>>8].life = 10;
				}
			}

	return 0;
}
