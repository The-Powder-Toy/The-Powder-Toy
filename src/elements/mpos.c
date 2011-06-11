#include <element.h>

int update_MPOS(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
    if((r&TYPE)==PT_BRMT){
        parts[r>>PS].vx = 0;
        parts[r>>PS].vy = 0;
        parts[r>>PS].ctype = PT_BRMT;
    }
    if(1>rand()%80){
        for (rx=-24; rx<24; rx++)
            for (ry=-24; ry<24; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if ((r&TYPE)==PT_BRMT){
                        if ((parts[r>>PS].ctype != PT_BRMT)||(parts[i].y<parts[r>>PS].y)){
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/10)*(rand()%5+10/10);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/10)*(rand()%5+10/10);
                    }else{
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/140)*(rand()%5+10/10);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/140)*(rand()%5+10/10);
					}
				}
            }
    }
    if((r&TYPE)==PT_IFIL){
        parts[r>>PS].vx = 0;
        parts[r>>PS].vy = 0;
        parts[r>>PS].ctype = PT_IFIL;
    }
    if(1>rand()%80){
        for (rx=-48; rx<48; rx++)
            for (ry=-48; ry<48; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if ((r&TYPE)==PT_IFIL){
                        if ((parts[r>>PS].ctype != PT_IFIL)||(parts[i].y<parts[r>>PS].y)){
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/20)*(rand()%5+20/20);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/10)*(rand()%5+20/20);
                        }else{
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/280)*(rand()%5+20/20);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/280)*(rand()%5+20/20);
                        }
                    }
                }
    }

	return 0;
}
