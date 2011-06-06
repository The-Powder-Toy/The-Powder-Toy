#include <element.h>

int update_SPMG(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
    if(r&TYPE==PT_BRMT){
        parts[r>>PS].vx = 0;
        parts[r>>PS].vy = 0;
        parts[r>>PS].ctype = PT_BRMT;
    }
    if(1>rand()%80){
        for (rx=-380; rx<380; rx++)
            for (ry=-380; ry<380; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if (r&TYPE==PT_BRMT){
                        if ((parts[r>>PS].ctype != PT_BRMT)||(parts[i].y<parts[r>>PS].y)){
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/100)*(rand()%5+100/100);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/100)*(rand()%5+100/100);
                    }else{
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/140)*(rand()%5+100/100);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/140)*(rand()%5+100/100);
					}
				}
            }
    }
    if(r&TYPE==PT_IFIL){
        parts[r>>PS].vx = 0;
        parts[r>>PS].vy = 0;
        parts[r>>PS].ctype = PT_IFIL;
    }
    if(1>rand()%80){
        for (rx=-480; rx<480; rx++)
            for (ry=-480; ry<480; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if (r&TYPE==PT_IFIL){
                        if ((parts[r>>PS].ctype != PT_IFIL)||(parts[i].y<parts[r>>PS].y)){
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/20)*(rand()%5+200/200);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/10)*(rand()%5+200/200);
                        }else{
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/28)*(rand()%5+2/200);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/28)*(rand()%5+2/200);
                        }
                    }
                }
    }

	return 0;
}
