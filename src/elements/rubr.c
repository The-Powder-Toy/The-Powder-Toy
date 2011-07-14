#include <element.h>

int update_RUBR(UPDATE_FUNC_ARGS)
{
    int r,rx,ry,t;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 &&
                    x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r || parts[r>>PS].type==PT_RUBR)
                    continue;
                parts[r>>PS].collision = -2.5f;
                parts[r>>PS].airdrag = 0;
            }
    for(rx=-4; rx<5; rx++)
        for(ry=-4; ry<5; ry++)
            if(x+rx>=0 && y+ry>0 &&
                    x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r || parts[r>>PS].type==PT_RUBR)
                    continue;
                    if (parts[i].x > parts[r>>PS].x + 5 || parts[i].x < parts[r>>PS].x - 5 || parts[i].y > parts[r>>PS].y + 5 || parts[i].y < parts[r>>PS].y - 5){
                        parts[r>>PS].collision = ptypes[parts[r>>PS].type].collision;
                        parts[r>>PS].airdrag = ptypes[parts[r>>PS].type].airdrag;
                    }
            }
}
