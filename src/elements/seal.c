#include <element.h>

int update_SEAL(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (pv[y/CELL][x/CELL] > parts[i].temp - 273.15)
                {
                    parts[i].type = PT_GOO;
                }
                if (parts[i].temp - 273.15 > MAXVELOCITY){
                    parts[i].temp = MAXVELOCITY + 273.15;
                } else if (parts[i].temp - 273.15 < MINVELOCITY){
                    parts[i].temp = MINVELOCITY + 273.15;
                }
            }
    return 0;
}
