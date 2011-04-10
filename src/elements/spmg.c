#include <element.h>

int update_SPMG(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
    if((r&0xFF)==PT_BRMT){
        parts[r>>8].vx = 0;
        parts[r>>8].vy = 0;
        parts[r>>8].ctype = PT_BRMT;
    }
    if(1>rand()%80){
        for (rx=-380; rx<380; rx++)
            for (ry=-380; ry<380; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART || !r)
                        continue;
                    if ((r&0xFF)==PT_BRMT){
                        if ((parts[r>>8].ctype != PT_BRMT)||(parts[i].y<parts[r>>8].y)){
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/100)*(rand()%5+100/100);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/100)*(rand()%5+100/100);
                    }else{
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/140)*(rand()%5+100/100);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/140)*(rand()%5+100/100);
					}
				}
            }
    }
    if((r&0xFF)==PT_IFIL){
        parts[r>>8].vx = 0;
        parts[r>>8].vy = 0;
        parts[r>>8].ctype = PT_IFIL;
    }
    if(1>rand()%80){
        for (rx=-480; rx<480; rx++)
            for (ry=-480; ry<480; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART || !r)
                        continue;
                    if ((r&0xFF)==PT_IFIL){
                        if ((parts[r>>8].ctype != PT_IFIL)||(parts[i].y<parts[r>>8].y)){
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/20)*(rand()%5+200/200);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/10)*(rand()%5+200/200);
                        }else{
                            parts[r>>8].vx = (((parts[i].x)-(parts[r>>8].x))/28)*(rand()%5+2/200);
                            parts[r>>8].vy = (((parts[i].y)-(parts[r>>8].y))/28)*(rand()%5+2/200);
                        }
                    }
                }
    }

	return 0;
}
