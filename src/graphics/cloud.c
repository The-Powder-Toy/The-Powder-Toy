#include <element.h>

int graphics_CLOUD(GRAPHIC_FUNC_ARGS)
{
    int x,y,cg,cb,cr,t;
    t = parts[i].type;
    if(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        if (parts[i].tmp2 == 2)
        {
            cg = PIXG(ptypes[t].pcolors);
            cb = PIXB(ptypes[t].pcolors);
            cr = PIXR(ptypes[t].pcolors);
        }
        else
        {
            cg = PIXG(ptypes[t].pcolors)/4;
            cb = PIXB(ptypes[t].pcolors)/4;
            cr = PIXR(ptypes[t].pcolors)/4;
        }
        cg += fire_g[y][x];
        if(cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if(cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if(cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
        fire_r[y][x] = cr;
    }
    else
    {
        for(x=-3; x<4; x++)
        {
            for(y=-3; y<4; y++)
            {
                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
                if (abs(x)+abs(y) == 2)
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
            }
        }
    }
    return 0;
}
