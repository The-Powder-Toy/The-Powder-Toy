#include <element.h>

int update_EQUALVEL(UPDATE_FUNC_ARGS) {
    int r,rx,ry;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>8)>=NPART || !r)
                    continue;
                parts[i].vx = parts[r>>8].vx;
                parts[i].vy = parts[r>>8].vy;
                if((r&0xFF)==PT_SAND){
                    parts[i].vx ++;
                    parts[i].vy ++;
                    parts[i].x = parts[r>>8].x;
                    parts[i].y = parts[r>>8].y;
                }
            }
	return 0;
}
