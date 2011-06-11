#include <element.h>

int update_EQUALVEL(UPDATE_FUNC_ARGS) {
    int r,rx,ry;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                parts[i].vx = parts[r>>PS].vx;
                parts[i].vy = parts[r>>PS].vy;
                if((r&TYPE)==PT_SAND){
                    parts[i].vx ++;
                    parts[i].vy ++;
                    parts[i].x = parts[r>>PS].x;
                    parts[i].y = parts[r>>PS].y;
                }
            }
	return 0;
}
