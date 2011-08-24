#include <element.h>
 
int update_ANAR(UPDATE_FUNC_ARGS) {
        int r, rx, ry;
       
        //if (parts[i].temp >= 0.23)
               // parts[i].temp --;
        for (rx=-2; rx<3; rx++)
                for (ry=-2; ry<3; ry++)
                        if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                        {
                                r = pmap[y+ry][x+rx];
                                if (!r)
                                        continue;
                                if ((r&0xFF)==PT_HFLM)
                                {
                                        if (1>rand()%22)
                                        {
                                                part_change_type(i,x,y,PT_HFLM);
                                                parts[i].life = rand()%150+50;
                                                parts[r>>8].temp = parts[i].temp = 0;
                                                pv[y/CELL][x/CELL] -= 0.5;
                                        }
                                }
                        }
        return 0;
}
