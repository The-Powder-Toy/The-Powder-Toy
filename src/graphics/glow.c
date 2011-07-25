#include <element.h>

int graphics_GLOW(GRAPHIC_FUNC_ARGS)
{
    int fr,fg,fb,cr,cg,cb,x,y,t;
    t = parts[i].type;
    if(parts[i].tmp2 == 2)
    {
        fr = 255;
        fg = 255;
        fb = 255;

        cr = 255;
        cg = 255;
        cb = restrict_flt(255, 255, 255);
    }
    else
    {
        fr = restrict_flt(parts[i].temp-(275.13f+32.0f), 0, 128)/50.0f;
        fg = restrict_flt(parts[i].ctype, 0, 128)/50.0f;
        fb = restrict_flt(parts[i].tmp, 0, 128)/50.0f;

        cr = restrict_flt(64.0f+parts[i].temp-(275.13f+32.0f), 0, 255);
        cg = restrict_flt(64.0f+parts[i].ctype, 0, 255);
        cb = restrict_flt(64.0f+parts[i].tmp, 0, 255);
    }


    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        fg += fire_g[y][x];
        if (fg > 255) fg = 255;
        fire_g[y][x] = fg;
        fb += fire_b[y][x];
        if (fb > 255) fb = 255;
        fire_b[y][x] = fb;
        fr += fire_r[y][x];
        if (fr > 255) fr = 255;
        fire_r[y][x] = fr;
    }
    if (cmode == CM_BLOB)
    {
        blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
        blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
        blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
        blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

        blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
        blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
        blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
        blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
    }
    return 0;
}
