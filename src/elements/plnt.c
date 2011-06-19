#include <element.h>

int update_PLNT(UPDATE_FUNC_ARGS)
{
    int r, rx, ry, np;
    for (rx=-2; rx<3; rx++)
        for (ry=-2; ry<3; ry++)
            if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if ((r>>8)>=NPART || !r)
                    continue;
                if ((r&0xFF)==PT_WATR && 1>(rand()%250))
                {
                    np = create_part(r>>8,x+rx,y+ry,PT_PLNT);
                    if (np<0) continue;
                    parts[np].life = 0;
                }
                if ((r&TYPE)==PT_RWTR && 1>(rand()%250))
                {
                    np = create_part(r>>8,x+rx,y+ry,PT_PLNT);
                    if (np<0) continue;
                    parts[np].life = 0;
                }
                else if ((r&0xFF)==PT_LAVA && 1>(rand()%250))
                {
                    part_change_type(i,x,y,PT_FIRE);
                    parts[i].life = 4;
                }
                else if ((r&0xFF)==PT_SMKE && (1>rand()%250))
                {
                    kill_part(r>>8);
                    parts[i].life = rand()%60 + 60;
                }
                else if ((r&TYPE)==PT_C02 && (1>rand()%250))
                {
                    kill_part(r>>PS);
                    parts[i].life = rand()%60 + 60;
                }
                else if(parts[r>>PS].type==PT_ROOT && parts[r>>PS].tmp2==1)
                {
                    parts[r>>PS].tmp2 = 2;
                    parts[i].ctype = PT_ROOT;
                }
                if (parts[i].ctype==PT_ROOT)
                {
                    int nrx = rand()%3 -1;
                    int nry = rand()%3 -1;
                    if (parts[r>>PS].type == PT_PLNT && parts[r>>PS].ctype != PT_ROOT && (1>rand()%50))
                        parts[r>>PS].ctype = PT_ROOT;

                    if(create_part(-1,x+rx+nrx,y+ry+nry,PT_PLNT))
                        parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                    if (1>rand()%2)
                        parts[i].ctype=PT_NONE;
                }
                else if ((r&0xFF)==PT_WOOD && (1>rand()%20) && abs(rx+ry)<=2 && VINE_MODE)
                {
                    int nnx = rand()%3 -1;
                    int nny = rand()%3 -1;
                    if (x+rx+nnx>=0 && y+ry+nny>0 && x+rx+nnx<XRES && y+ry+nny<YRES && (nnx || nny))
                    {
                        if ((pmap[y+ry+nny][x+rx+nnx]>>8)>=NPART||pmap[y+ry+nny][x+rx+nnx])
                            continue;
                        np = create_part(-1,x+rx+nnx,y+ry+nny,PT_VINE);
                        if (np<0) continue;
                        parts[np].temp = parts[i].temp;
                    }
                }
                else if(parts[r>>PS].type==PT_DIRT && (1>rand()%20))
                {
                    int nrx = rand()%3 -1;
                    int nry = rand()%3 -1;
                    if(x+rx+nrx>=0 && y+ry+nry>0 &&
                            x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                    {
                        if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                            continue;
                        if(create_part(-1,x+rx+nrx,y+ry+nry,PT_GRAS))
                            parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                    }
                }
            }
    if (parts[i].life==2)
    {
        for (rx=-1; rx<2; rx++)
            for (ry=-1; ry<2; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if ((r>>8)>=NPART)
                        continue;
                    if (!r)
                        create_part(-1,x+rx,y+ry,PT_O2);
                }
        parts[i].life = 0;
    }
    return 0;
}
