#include <element.h>

int graphics_effect_GLOW(GRAPHIC_FUNC_ARGS)
{
    int fg,fb,fr,x,y,t;
    t = parts[i].type;
    fg = PIXG(ptypes[t].pcolors);
    fb = PIXB(ptypes[t].pcolors);
    fr = PIXR(ptypes[t].pcolors);
    /*if(pv[ny/CELL][nx/CELL]>0)
    {
        fg = PIXG(ptypes[t].pcolors) * pv[ny/CELL][nx/CELL];
        fb = PIXB(ptypes[t].pcolors) * pv[ny/CELL][nx/CELL];
        fr = PIXR(ptypes[t].pcolors) * pv[ny/CELL][nx/CELL];
    }*/
    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB((int)restrict_flt(0x44 + fr, 0, 255), (int)restrict_flt(0x88 + fg*8, 0, 255), (int)restrict_flt(0x44 + fb*8, 0, 255));
    if(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        fg += fire_g[y][x];
        if(fg > PIXG(ptypes[t].pcolors)) fg = PIXG(ptypes[t].pcolors);
        fire_g[y][x] = fg;
        fb += fire_b[y][x];
        if(fb > PIXB(ptypes[t].pcolors)) fb = PIXB(ptypes[t].pcolors);
        fire_b[y][x] = fb;
        fr += fire_r[y][x];
        if(fr > PIXR(ptypes[t].pcolors)) fr = PIXR(ptypes[t].pcolors);
        fire_r[y][x] = fr;
    }
    if(cmode == CM_BLOB)
    {
        uint8 R = (int)restrict_flt(0x44 + fr*8, 0, 255);
        uint8 G = (int)restrict_flt(0x88 + fg*8, 0, 255);
        uint8 B = (int)restrict_flt(0x44 + fb*8, 0, 255);

        blendpixel(vid, nx+1, ny, R, G, B, 223);
        blendpixel(vid, nx-1, ny, R, G, B, 223);
        blendpixel(vid, nx, ny+1, R, G, B, 223);
        blendpixel(vid, nx, ny-1, R, G, B, 223);

        blendpixel(vid, nx+1, ny-1, R, G, B, 112);
        blendpixel(vid, nx-1, ny-1, R, G, B, 112);
        blendpixel(vid, nx+1, ny+1, R, G, B, 112);
        blendpixel(vid, nx-1, ny+1, R, G, B, 112);
    }
    return 1;
}
#include <element.h>

int graphics_effect_GAS(GRAPHIC_FUNC_ARGS)
{
    int t,cb,cg,cr,x,y;
    t = parts[i].type;
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        cg = PIXG(ptypes[t].pcolors)/3;
        cb = PIXB(ptypes[t].pcolors)/3;
        cr = PIXR(ptypes[t].pcolors)/3;
        cg += fire_g[y][x];
        if (cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if (cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if (cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
        fire_r[y][x] = cr;
    }
    else
    {
        for (x=-3; x<4; x++)
        {
            for (y=-3; y<4; y++)
            {
                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
                if (abs(x)+abs(y) <=3 && abs(x)+abs(y))
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
                if (abs(x)+abs(y) == 2)
                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
            }
        }
    }
    return 1;
}
