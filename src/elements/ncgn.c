#include <element.h>

int update_NCGN(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	int self = parts[i].type;
    for(rx=-1; rx<2; rx++)
        for(ry=-1; ry<2; ry++)
            if(x+rx>=0 && y+ry>0 &&
               x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_PLUT){
                    parts[i].ctype=self;
                    parts[i].life = 4;
                    part_change_type(i,x,y,PT_SPRK);
                    part_change_type(r>>PS,x+rx,y+ry,PT_NCWS);
                }
            }
	return 0;
}
