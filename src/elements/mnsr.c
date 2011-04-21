#include <element.h>

int update_MNSR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
                //OLD CODE
                /*
				create_part(-1, x+1, y, PT_SAND);
                create_part(-1, x+1, y+1, PT_SAND);
                create_part(-1, x, y+1, PT_SAND);
                create_part(-1, x-1, y+1, PT_SAND);
                create_part(-1, x-1, y, PT_SAND);
                create_part(-1, x-1, y-1, PT_SAND);
                create_part(-1, x, y-1, PT_SAND);
                create_part(-1, x+1, y-1, PT_SAND);
                parts[i].type=PT_EQUALVEL;
                 */
                create_part(-1, x+1, y, PT_SAND);
                create_part(-1, x+1, y+1, PT_SAND);
                create_part(-1, x, y+1, PT_SAND);
                create_part(-1, x-1, y+1, PT_SAND);
                create_part(-1, x-1, y, PT_SAND);
                create_part(-1, x-1, y-1, PT_SAND);
                create_part(-1, x, y-1, PT_SAND);
                create_part(-1, x+1, y-1, PT_SAND);
                part_change_type(i,x,y,PT_EQUALVEL);
			}
	return 0;
}
