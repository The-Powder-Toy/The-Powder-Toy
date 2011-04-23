#include <element.h>

int update_NMTR(UPDATE_FUNC_ARGS) {
    int r, rx, ry;
	for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>8)>=NPART || !r)
                    continue;
                if ((r&0xFF)==PT_NEUT){
                    parts[i].temp = MAX_TEMP;
                    parts[i].type = PT_PLSM;
                }
                if (parts[i].temp>3500)
                {
                create_part(-1,x,y,PT_PLSM);
                parts[i].temp=MAX_TEMP;
                part_change_type(i,x,y,PT_PLSM);
                pv[y/CELL][x/CELL] += 20.0f * CFDS;
            }
            parts[i].vy = parts[i].temp/100;
        }
	return 0;
}
