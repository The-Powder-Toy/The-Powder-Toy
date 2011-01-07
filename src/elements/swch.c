#include <powder.h>

int update_SWCH(UPDATE_FUNC_ARGS) {
	int r, rt;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts_avg(i,r>>8,PT_INSL)!=PT_INSL) {
					rt = parts[r>>8].type; // not r&0xFF because pmap is not yet always updated (TODO)
					if (rt==PT_SWCH)
					{
						if (parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
							parts[i].life = 9;
						else if (parts[i].life==0&&parts[r>>8].life==10)
							parts[i].life = 10;
					}
					else if (rt==PT_SPRK&&parts[i].life==10&&parts[r>>8].ctype!=PT_PSCN&&parts[r>>8].ctype!=PT_NSCN) {
						parts[i].type = PT_SPRK;
						parts[i].ctype = PT_SWCH;
						parts[i].life = 4;
					}
				}
			}
	return 0;
}
