#include <element.h>

int update_VIRS(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
	int self = parts[i].type;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 &&
               x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if(parts[r>>PS].type!=PT_DMND && parts[r>>PS].type!=PT_GOLD && parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_CNVR && parts[r>>PS].type!=PT_VOID && parts[r>>PS].type!=PT_WHOL && parts[r>>PS].type!=PT_BHOL)
                {
                    if (1>rand()%25)
                        part_change_type(r>>PS,x+rx,y+ry,self);
                }
            }
    return 0;
}
