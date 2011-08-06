#include <element.h>

int update_HETR(UPDATE_FUNC_ARGS)
{
    int r, rx, ry;
    if (parts[i].life>0 && parts[i].life!=10)
        parts[i].life--;
    if (parts[i].tmp>0 && parts[i].tmp!=10)
        parts[i].tmp--;
    int self = parts[i].type;
        for (rx=-1; rx<2; rx++)
            for (ry=-1; ry<2; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if (parts[i].tmp==0 && parts[i].life==10)
                    {
                        parts[r>>PS].temp += 2;
                        parts[i].temp = 373.15;
                        if(aheat_enable)
                            hv[y/CELL][x/CELL] += 2;
                    }
                    else if (parts[i].tmp > 0 && parts[i].life > 0)
                    {
                        parts[r>>PS].temp -= 2;
                        parts[i].temp = 0;
                        if(aheat_enable)
                            hv[y/CELL][x/CELL] -= 2;
                    }
                    if ((r&TYPE)==self && parts[i].life==10)
                    {
                        if (parts[r>>PS].life<10&&parts[r>>PS].life>0)
                            parts[i].life = 9;
                        else if (parts[r>>PS].life==0)
                            parts[r>>PS].life = 10;
                    }
                    if ((r&TYPE)==self && parts[i].tmp==10)
                    {
                        if (parts[r>>PS].tmp<10&&parts[r>>PS].tmp>0)
                            parts[i].tmp = 9;
                        else if (parts[r>>PS].tmp==0)
                            parts[r>>PS].tmp = 10;
                    }

                }
    return 0;
}
