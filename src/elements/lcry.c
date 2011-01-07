#include <powder.h>

int update_LCRY(UPDATE_FUNC_ARGS) {
	int r;
	if (parts[i].life==10)
	{
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 &&
				        x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r)
						continue;
					if ((r&0xFF)==PT_LCRY)
					{
						if (parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[r>>8].life==0)
							parts[r>>8].life = 10;
					}
				}
	}
	return 0;
}
