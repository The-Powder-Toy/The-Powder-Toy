#include "element.h"

int update_CAUS(UPDATE_FUNC_ARGS) {
	int r, rx, ry, trade, np;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)!=PT_ACID && (r&0xFF)!=PT_CAUS)
				{
					if (((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && sim->ptypes[r&0xFF].hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (sim->parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
							float newtemp = ((60.0f-(float)sim->ptypes[r&0xFF].hardness))*7.0f;
							if(newtemp < 0){
								newtemp = 0;
							}
							parts[i].temp += newtemp;
							parts[i].life--;
							sim->kill_part(r>>8);
						}
					}
					else if (parts[i].life<=50)
					{
						sim->kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}
