#include <element.h>

int update_SAND(UPDATE_FUNC_ARGS) {
	int r, nnx, nny, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if(r&TYPE==PT_EQUALVEL){
					parts[r>>PS].vx = 0;
					parts[r>>PS].vy = 0;
					parts[r>>PS].ctype = PT_EQUALVEL;
				}
			}
    for (rx=-24; rx<24; rx++)
        for (ry=-24; ry<24; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (r&TYPE==PT_EQUALVEL){
                    if ((parts[r>>PS].ctype != PT_EQUALVEL)||(parts[i].y<parts[r>>PS].y)){
                        parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/10)*(rand()%5+10/10);
                        parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/10)*(rand()%5+10/10);
                    }else{
                        parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/140)*(rand()%5+10/10);
                        parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/140)*(rand()%5+10/10);
                        parts[i].vx = parts[r>>PS].vx;
                        parts[i].vy = parts[r>>PS].vy;
                    }
                }
            }
	return 0;
}
