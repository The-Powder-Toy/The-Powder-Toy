#include <element.h>

int graphics_PHOT(GRAPHIC_FUNC_ARGS)
{
    int t,cb,cg,cr,x,y;
    t = parts[i].type;
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        cg = 0;
        cb = 0;
        cr = 0;
        for (x=0; x<12; x++)
        {
            cr += (parts[i].ctype >> (x+18)) & 1;
            cb += (parts[i].ctype >>  x)     & 1;
        }
        for (x=0; x<14; x++)
            cg += (parts[i].ctype >> (x+9))  & 1;
        x = 624/(cr+cg+cb+1);
        cr *= x;
        cg *= x;
        cb *= x;
        vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr>255?255:cr,cg>255?255:cg,cb>255?255:cb);
        cr >>= 4;
        cg >>= 4;
        cb >>= 4;
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
        cg = 0;
        cb = 0;
        cr = 0;
        for (x=0; x<12; x++)
        {
            cr += (parts[i].ctype >> (x+18)) & 1;
            cb += (parts[i].ctype >>  x)     & 1;
        }
        for (x=0; x<14; x++)
            cg += (parts[i].ctype >> (x+9))  & 1;
        x = 624/(cr+cg+cb+1);
        cr *= x;
        cg *= x;
        cb *= x;
        cr = cr>255?255:cr;
        cg = cg>255?255:cg;
        cb = cb>255?255:cb;
        if(cmode == CM_PERS)
        {
            if(parts[i].pavg[0] && parts[i].pavg[1])
            {
                draw_line(vid, nx, ny, parts[i].pavg[0], parts[i].pavg[1], cr, cg, cb, XRES+BARSIZE);
            }
            else
            {
                vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(cr, cg, cb);
            }
        }
        else
        {
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
    }
    return 1;
}
