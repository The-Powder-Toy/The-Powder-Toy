#include <element.h>
int update_PMIC(UPDATE_FUNC_ARGS) 
{
	int r, rx, ry;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>8)>=NPART || !r)
							continue;
                        if ((r&0xFF)==PT_WATR && parts[r>>8].y < parts[i].y){
                            parts[i].y--;
                        }
                        if ((r&0xFF)==PT_RWTR && parts[r>>8].y < parts[i].y){
                            parts[i].y--;
                        }
                        if ((r&0xFF)==PT_DSTW && parts[r>>8].y < parts[i].y){
                            parts[i].y--;
                        }
                        if ((r&0xFF)==PT_SLTW && parts[r>>8].y < parts[i].y){
                            parts[i].y--;
                        }
					}
    return 0;
}
