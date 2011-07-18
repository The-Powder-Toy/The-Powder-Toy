#include <element.h>

int update_DICE(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
	int self = parts[i].type;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_WATR){
                    create_part(-1,x-1,y+1,PT_SMKE);
                    create_part(-1,x+1,y+1,PT_SMKE);
                }
                if (parts[r>>PS].type==PT_ICEI){
                    parts[r>>PS].type=PT_WATR;
                }
            }
    return 0;
}
