#include <element.h>

int update_VOLT(UPDATE_FUNC_ARGS)
{
    int self = parts[i].type;
    int r, rx, ry;
    create_part(-1, x, y-5, PT_LAVA);
    for (rx=-1; rx<2; rx++)
        for (ry=-1; ry<2; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_MGMA)
                {
                    part_change_type(r>>PS,x+rx,y+ry,PT_LAVA);
                }
            }
    return 0;
}
int update_VOLB(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    for (rx=-1; rx<2; rx++)
        for (ry=-1; ry<2; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_STNE || parts[r>>PS].type==PT_LAVA && parts[r>>PS].ctype==PT_STNE)
                {
                    parts[r>>PS].temp += 1000.0f;
                    parts[i].temp += 1000.0f;
                }
            }
    return 0;
}
