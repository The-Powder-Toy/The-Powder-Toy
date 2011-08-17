#include <element.h>

int graphics_PLSM(GRAPHIC_FUNC_ARGS)
{
    int fr,fg,fb,cr,cg,cb,x,y,t,ct;
    t = parts[i].type;
    ct = parts[i].ctype;
    uint8 R,G,B;
    if (parts[i].ctype == PT_NBLE || parts[i].ctype == PT_ARGN || parts[i].ctype == PT_HLIM || parts[i].ctype == PT_RDON || parts[i].ctype == PT_XNON || parts[i].ctype == PT_KPTN)
    {
        cr = PIXR(ptypes[ct].pcolors);
        cg = PIXG(ptypes[ct].pcolors);
        cb = PIXB(ptypes[ct].pcolors);
    }
    else
    {
        float ttemp = (float)parts[i].life;
        int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
        R = plasma_data[caddress];
        G = plasma_data[caddress+1];
        B = plasma_data[caddress+2];
    }
    if (cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        if (parts[i].ctype != PT_NBLE && parts[i].ctype != PT_RDON && parts[i].ctype != PT_HLIM && parts[i].ctype != PT_ARGN && parts[i].ctype != PT_XNON && parts[i].ctype != PT_KPTN)
        {
            cr = R/8;
            cg = G/8;
            cb = B/8;
            R = 255;
            G = 255;
            B = 255;
        }
        else
        {
            R = PIXR(ptypes[ct].pcolors);
            G = PIXG(ptypes[ct].pcolors);
            B = PIXB(ptypes[ct].pcolors);
            cr = R/8;
            cg = G/8;
            cb = B/8;
        }
        x = nx/CELL;
        y = ny/CELL;
        cg += fire_g[y][x];
        if (cg > G) cg = G;
        fire_g[y][x] = cg;
        cb += fire_b[y][x];
        if (cb > B) cb = B;
        fire_b[y][x] = cb;
        cr += fire_r[y][x];
        if (cr > R) cr = R;
        fire_r[y][x] = cr;
    }
    else
    {
        if (parts[i].ctype != PT_NBLE || parts[i].ctype != PT_ARGN|| parts[i].ctype != PT_HLIM|| parts[i].ctype != PT_RDON|| parts[i].ctype != PT_XNON|| parts[i].ctype != PT_KPTN)
        {
            cr = R;
            cg = G;
            cb = B;
        }
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
