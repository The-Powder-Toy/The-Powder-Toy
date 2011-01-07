#include <powder.h>

int update_BTRY(UPDATE_FUNC_ARGS) {
	int r, rt;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				rt = parts[r>>8].type;
				if (parts_avg(i,r>>8,PT_INSL) != PT_INSL)
				{
					if ((ptypes[rt].properties&PROP_CONDUCTS) && !(rt==PT_WATR||rt==PT_SLTW||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR) && parts[r>>8].life==0 && abs(nx)+abs(ny) < 4)
					{
						parts[r>>8].life = 4;
						parts[r>>8].ctype = rt;
						parts[r>>8].type = PT_SPRK;
					}
				}
			}
	return 0;
}
