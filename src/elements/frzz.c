#include <powder.h>

int update_FRZZ(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_WATR&&5>rand()%100)
				{
					parts[r>>8].type=PT_FRZW;
					parts[r>>8].life = 100;
					parts[i].type = PT_NONE;
				}

			}
	return 0;
}
