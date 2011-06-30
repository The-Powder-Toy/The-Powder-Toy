#include <element.h>

int update_GEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, trade, np;
	if(parts[r>>PS].type==PT_GEL){
        parts[r>>PS].vx = 0;
        parts[r>>PS].vy = 0;
        parts[r>>PS].ctype = PT_GEL;
    }
	for (rx=-5; rx<5; rx++)
            for (ry=-5; ry<5; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry)){
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if (parts[r>>PS].type==PT_GEL){
                        if ((parts[r>>PS].ctype != PT_GEL)||(parts[i].y<parts[r>>PS].y)){
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/100)*(rand()%5+100/100);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/100)*(rand()%5+100/100);
                    }else{
                            parts[r>>PS].vx = (((parts[i].x)-(parts[r>>PS].x))/140)*(rand()%5+100/100);
                            parts[r>>PS].vy = (((parts[i].y)-(parts[r>>PS].y))/140)*(rand()%5+100/100);
					}
				}
            }
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART)
						continue;
					if (!r)
                    {
                        if (parts[i].vx > 3)
                            parts[i].vx = 3;
                        if (parts[i].vx < -3)
                            parts[i].vx = -3;
                        if (parts[i].vy > 3)
                            parts[i].vy = 3;
                        if (parts[i].vy < -3)
                            parts[i].vy = -3;
                    }
                    else
                    {
                        if (parts[i].vx > 0.1)
                            parts[i].vx = 0.1;
                        if (parts[i].vx < -0.1)
                            parts[i].vx = -0.1;
                        if (parts[i].vy > 0.1)
                            parts[i].vy = 0.1;
                        if (parts[i].vy < -0.1)
                            parts[i].vy = -0.1;
                    }
				}

	return 0;
}
