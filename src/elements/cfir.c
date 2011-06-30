#include <element.h>

int update_CFIR(UPDATE_FUNC_ARGS)
{
    int r,rx,ry, trade;
    rx = rand()%3-1;
    ry = rand()%3-1;
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type!=PT_WARP&&parts[r>>PS].type!=PT_POT&&parts[r>>PS].type!=PT_STKM&&parts[r>>PS].type!=PT_STKM2&&parts[r>>PS].type!=PT_DMND&&parts[r>>PS].type!=PT_CLNE&&parts[r>>PS].type!=PT_BCLN&&parts[r>>PS].type!=PT_PCLN&&(10>=rand()%2))
                {
                    int nrx = rand()%3 -1;
                    int nry = rand()%3 -1;
                    if(x+rx+nrx>=0 && y+ry+nry>0 &&
                            x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                    {
                        if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                            continue;
                        if (parts[r>>PS].type!=PT_CFIR)
                        {
                            create_part(-1, x+rx+nrx, y+ry+nry, PT_WARP);
                        }
                        else if (parts[r>>PS].type==PT_CFIR)
                        {
                            parts[i].x = rand()%3;
                            parts[i].x = rand()%-3;
                            parts[i].y = rand()%3;
                            parts[i].y = rand()%-3;
                        }
                        else
                        {
                            parts[i].vy = 2;
                        }
                    }
                    parts[i].life += 100;
                    create_part(-1, x, y, PT_CFIR);
                }
            }
    return 0;
}
