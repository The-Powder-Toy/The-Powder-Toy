#include <element.h>

int update_LTNG(UPDATE_FUNC_ARGS) {
	int r, rx, ry, nb;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
                    if ((ptypes[r&TYPE].properties&PROP_CONDUCTS) && parts[r>>PS].life==0 && parts[r>>PS].ctype!=PT_SPRK)
					{
                        kill_part(i);
                        parts[r>>PS].ctype = parts[r>>PS].type;
                        part_change_type(r>>PS,x+rx,y+ry,PT_SPRK);
                        parts[r>>PS].life = 4;
                    }
                }
	return 0;
}
