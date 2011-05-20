#include <element.h>

int update_NBHL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	gravmap[y/CELL][x/CELL] += 0.1f;

	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)!=PT_NBHL)
				{
					if((r&0xFF)<PT_NUM && !(ptypes[(r&0xFF)].properties & TYPE_SOLID))
						kill_part(r>>8);
				}
			}
	return 0;
}
