#include <element.h>

int graphics_HFLM(GRAPHIC_FUNC_ARGS)
{
    int fr,fg,fb,cr,cg,cb,x,y,t;
    t = parts[i].type;
    float ttemp = (float)parts[i].life;
    int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
    uint8 R = hflm_data[caddress];
    uint8 G = hflm_data[caddress+1];
    uint8 B = hflm_data[caddress+2];
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        cr = R/8;
        cg = G/8;
        cb = B/8;
        x = nx/CELL;
        y = ny/CELL;
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
        cr = R;
        cg = G;
        cb = B;
        blendpixel(vid, nx, ny, cr, cg, cb, 192);
        blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
        blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
        blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
        blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
        blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
        blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
        blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
        blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
    }
    return 1;
}
