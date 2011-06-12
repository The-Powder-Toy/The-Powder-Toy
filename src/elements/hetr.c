#include <element.h>

int update_HETR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
                    if (parts[i].tmp && parts[r>>PS].type==PT_HETR){
                        parts[r>>PS].tmp = parts[i].tmp;
                    }
                    if (parts[i].tmp==1){
                        parts[r>>PS].temp += 2;
                        parts[i].temp = 373.15;
                        if(aheat_enable)
                            hv[y/CELL][x/CELL] += 2;
                    } else if (parts[i].tmp==2){
                        parts[r>>PS].temp -= 2;
                        parts[i].temp = 0;
                        if(aheat_enable)
                            hv[y/CELL][x/CELL] -= 2;
                    }

				}
	return 0;
}
