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
            cg = PIXG(parts[i].dcolour);
            cb = PIXB(parts[i].dcolour);
            cr = PIXR(parts[i].dcolour);
        }
        else
        {
            cg = PIXG(parts[i].dcolour)/4;
            cb = PIXB(parts[i].dcolour)/4;
            cr = PIXR(parts[i].dcolour)/4;
        }
        cg += fire_g[y][x];
        if(cg > PIXG(parts[i].dcolour)/2) cg = PIXG(parts[i].dcolour)/2;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if(cb > PIXB(parts[i].dcolour)/2) cb = PIXB(parts[i].dcolour)/2;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if(cr > PIXR(parts[i].dcolour)/2) cr = PIXR(parts[i].dcolour)/2;
        fire_r[y][x] = cr;
    }
    else
    {
        for(x=-3; x<4; x++)
        {
            for(y=-3; y<4; y++)
            {
                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                    blendpixel(vid,x+nx,y+ny, PIXR(parts[i].dcolour)/1.6, PIXG(parts[i].dcolour)/1.6, PIXB(parts[i].dcolour)/1.6, 30);
                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                    blendpixel(vid,x+nx,y+ny, PIXR(parts[i].dcolour)/1.6, PIXG(parts[i].dcolour)/1.6, PIXB(parts[i].dcolour)/1.6, 10);
                if (abs(x)+abs(y) == 2)
                    blendpixel(vid,x+nx,y+ny, PIXR(parts[i].dcolour)/1.6, PIXG(parts[i].dcolour)/1.6, PIXB(parts[i].dcolour)/1.6, 20);
            }
        }
    }
    return 0;
}
