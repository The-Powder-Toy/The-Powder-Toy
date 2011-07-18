#include <element.h>
int update_PMIC(UPDATE_FUNC_ARGS)
{
	int r, rx, ry;
	int self = parts[i].type;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>PS)>=NPART || !r)
							continue;
                        if (parts[r>>PS].type==PT_WATR && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (parts[r>>PS].type==PT_RWTR && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (parts[r>>PS].type==PT_DSTW && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
                        if (parts[r>>PS].type==PT_SLTW && parts[r>>PS].y < parts[i].y){
                            parts[i].y--;
                        }
					}
    return 0;
}
