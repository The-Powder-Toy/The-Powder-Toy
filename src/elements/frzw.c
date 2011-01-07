#include <powder.h>

int update_FRZW(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_WATR&&5>rand()%70)
				{
					parts[r>>8].type=PT_FRZW;
				}
			}
	if (parts[i].life==0&&13>rand()%2500)
	{
		parts[i].type=PT_ICEI;
		parts[i].ctype=PT_FRZW;
		parts[i].temp -= 200.0f;
		if (parts[i].temp<0)
			parts[i].temp = 0;
	}
	else if ((100-(parts[i].life))>rand()%50000)
	{
		parts[i].type=PT_ICEI;
		parts[i].ctype=PT_FRZW;
		parts[i].temp -= 200.0f;
		if (parts[i].temp<0)
			parts[i].temp = 0;
	}
	return 0;
}
