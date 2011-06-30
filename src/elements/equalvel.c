#include <element.h>

int update_EQUALVEL(UPDATE_FUNC_ARGS) {
    int r,rx,ry;
    /*for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                parts[i].vx = parts[r>>PS].vx;
                parts[i].vy = parts[r>>PS].vy;
                if(parts[r>>PS].type==PT_SAND){
                    parts[i].vx ++;
                    parts[i].vy ++;
                    parts[i].x = parts[r>>PS].x;
                    parts[i].y = parts[r>>PS].y;
                }
            }*/
    for(rx=-3; rx<4; rx++)
        for(ry=-3; ry<4; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (rx == -3 && ry == -3 || parts[i].num == 0){
                    parts[i].a1 = parts[r>>PS].vx;
                    parts[i].ay1 = parts[r>>PS].vy;
                    parts[i].num = 1;
                } else if (parts[i].num > 9) {
                    if (parts[i].num==2){
                        parts[i].a2 = parts[r>>PS].vx;
                        parts[i].ay2 = parts[r>>PS].vy;
                    } else if (parts[i].num==3){
                        parts[i].a3 = parts[r>>PS].vx;
                        parts[i].ay3 = parts[r>>PS].vy;
                    } else if (parts[i].num==4){
                        parts[i].a4 = parts[r>>PS].vx;
                        parts[i].ay4 = parts[r>>PS].vy;
                    } else if (parts[i].num==5){
                        parts[i].a5 = parts[r>>PS].vx;
                        parts[i].ay5 = parts[r>>PS].vy;
                    } else if (parts[i].num==6){
                        parts[i].a6 = parts[r>>PS].vx;
                        parts[i].ay6 = parts[r>>PS].vy;
                    } else if (parts[i].num==7){
                        parts[i].a7 = parts[r>>PS].vx;
                        parts[i].ay7 = parts[r>>PS].vy;
                    } else if (parts[i].num==8){
                        parts[i].a8 = parts[r>>PS].vx;
                        parts[i].ay8 = parts[r>>PS].vy;
                    }
                    parts[i].num++;
                } else {
                    int tmpx = parts[i].a1 + parts[i].a2 + parts[i].a3 + parts[i].a4 + parts[i].a5 + parts[i].a6 + parts[i].a7 + parts[i].a8;
                    int tmpy = parts[i].ay1 + parts[i].ay2 + parts[i].ay3 + parts[i].ay4 + parts[i].ay5 + parts[i].ay6 + parts[i].ay7 + parts[i].ay8;
                    parts[i].vx = tmpx / 8;
                    parts[i].vy = tmpy / 8;
                    parts[i].num = 0;
                }
            }
	return 0;
}
