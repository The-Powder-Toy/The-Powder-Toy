#include <element.h>
int update_PMIC(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>PS)>=NPART || !r)
							continue;
                        if (r&TYPE==PT_WATR && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (r&TYPE==PT_RWTR && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (r&TYPE==PT_DSTW && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (r&TYPE==PT_SLTW && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
					}
    return 0;
}
