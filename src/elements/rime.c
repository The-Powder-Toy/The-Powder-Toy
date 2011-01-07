#include <powder.h>

int update_RIME(UPDATE_FUNC_ARGS) {
	int r, rt;
	parts[i].vx = 0;
	parts[i].vy = 0;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				rt = parts[r>>8].type;
				if (rt==PT_SPRK)
				{
					parts[i].type = PT_FOG;
					parts[i].life = rand()%50 + 60;
				}
				else if (rt==PT_FOG&&parts[r>>8].life>0)
				{
					parts[i].type = PT_FOG;
					parts[i].life = parts[r>>8].life;
				}
			}
	return 0;
}
