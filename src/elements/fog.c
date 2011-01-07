#include <powder.h>

int update_FOG(UPDATE_FUNC_ARGS) {
	int r;
	if (parts[i].temp>=373.15)
		parts[i].type = PT_WTRV;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (pstates[parts[r>>8].type].state==ST_SOLID&&5>=rand()%50&&parts[i].life==0&&!(parts[r>>8].type==PT_CLNE||parts[r>>8].type==PT_PCLN))
				{
					parts[i].type = PT_RIME;
				}
				if (parts[r>>8].type==PT_SPRK)
				{
					parts[i].life += rand()%20;
				}
			}
	return 0;
}
