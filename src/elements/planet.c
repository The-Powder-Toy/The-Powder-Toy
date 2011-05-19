#include <element.h>

int update_PLAN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>8)>=NPART || !r)
                    continue;
            }

	if (ngrav_enable==1){
        gravmap[y/CELL][x/CELL] += parts[i].tmp;
    }
	return 0;
}
