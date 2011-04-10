#include <element.h>

int update_EQUALVEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if((r&0xFF)==PT_SAND){
					parts[r>>8].vx = 0;
					parts[r>>8].vy = 0;
					parts[r>>8].ctype = PT_SAND;
				}
			}
        for (rx=-24; rx<24; rx++)
            for (ry=-24; ry<24; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART || !r)
                        continue;
                    if ((r&0xFF)==PT_SAND){
                        if ((parts[r>>8].ctype != PT_SAND)||(parts[i].y<parts[r>>8].y)){
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/10)*(rand()%5+10/10);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/10)*(rand()%5+10/10);
                        }else{
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/140)*(rand()%5+10/10);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/140)*(rand()%5+10/10);
                        }
                    }
                }
	return 0;
}
