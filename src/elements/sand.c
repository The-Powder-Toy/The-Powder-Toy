#include <element.h>

int update_SAND(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if((r&0xFF)==PT_EQUALVEL){
					parts[r>>8].vx = 0;
					parts[r>>8].vy = 0;
					parts[r>>8].ctype = PT_EQUALVEL;
				}
			}
    for (rx=-24; rx<24; rx++)
        for (ry=-24; ry<24; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                r = pmap[y+ry][x+rx];
                if ((r>>8)>=NPART || !r)
                    continue;
                if ((r&0xFF)==PT_EQUALVEL){
                    if ((parts[r>>8].ctype != PT_EQUALVEL)||(parts[i].y<parts[r>>8].y)){
                        parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/10)*(rand()%5+10/10);
                        parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/10)*(rand()%5+10/10);
                    }else{
                        parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/140)*(rand()%5+10/10);
                        parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/140)*(rand()%5+10/10);
                        parts[i].vx = parts[r>>8].vx;
                        parts[i].vy = parts[r>>8].vy;
                    }
                }
            }
	return 0;
}
