#include <element.h>

int graphics_O3(GRAPHIC_FUNC_ARGS)
{
    int t,cb,cg,cr,x,y;
    t = parts[i].type;
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        cg = 166;
        cb = 175;
        cr = 125;
        cg += fire_g[y][x];
        if (cg > 166) cg = 166;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if (cb > 175) cb = 175;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if (cr > 125) cr = 125;
        fire_r[y][x] = cr;
    }
    else
    {
        for (x=-3; x<4; x++)
        {
            for (y=-3; y<4; y++)
            {
                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                    blendpixel(vid,x+nx,y+ny,100,100,100,30);
                if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
                    blendpixel(vid,x+nx,y+ny,100,100,100,10);
                if (abs(x)+abs(y) == 2)
                    blendpixel(vid,x+nx,y+ny,100,100,100,20);
            }
        }
    }
    return 0;
}
