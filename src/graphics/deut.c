#include <element.h>

int graphics_DEUT(GRAPHIC_FUNC_ARGS)
{
    int x,y,cr,cg,cb,t;
    t = parts[i].type;
    if (parts[i].life>=700&&(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE))
    {
        x = nx/CELL;
        y = ny/CELL;
        cr = 20;
        cg = 20;
        cb = 20;
        cg += fire_g[y][x];
        if (cg > 255) cg = 255;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if (cb > 255) cb = 255;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if (cr > 255) cr = 255;
        fire_r[y][x] = cr;
    }
    else
    {
        cr = PIXR(ptypes[t].pcolors) + parts[i].life*1;
        cg = PIXG(ptypes[t].pcolors) + parts[i].life*2;
        cb = PIXB(ptypes[t].pcolors) + parts[i].life*4;
        if (cr>=255)
            cr = 255;
        if (cg>=255)
            cg = 255;
        if (cb>=255)
            cb = 255;
        blendpixel(vid, nx, ny, cr, cg, cb, 255);

    }
    return 1;
}
