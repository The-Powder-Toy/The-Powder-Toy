#include <powder.h>

int update_ACID(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)!=PT_ACID)
				{
					if ((r&0xFF)==PT_PLEX || (r&0xFF)==PT_NITR || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD)
					{
						parts[i].type = PT_FIRE;
						parts[i].life = 4;
						parts[r>>8].type = PT_FIRE;
						parts[r>>8].life = 4;
					}
					else if (((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && ptypes[parts[r>>8].type].hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)
						{
							parts[i].life--;
							parts[r>>8].type = PT_NONE;
							return 1;
						}
					}
					else if (parts[i].life<=50)
					{
						parts[i].life = 0;
						parts[i].type = PT_NONE;
						return 1;
					}
				}
			}
	return 0;
}
