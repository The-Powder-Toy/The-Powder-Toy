#include <element.h>

int graphics_ACID(GRAPHIC_FUNC_ARGS)
{
    int s,cr,cg,cb;
    if (parts[i].life>75) parts[i].life = 75;
    if (parts[i].life<49) parts[i].life = 49;
    s = (parts[i].life-49)*3;
    if (s==0) s = 1;
    cr = 0x86 + s*4;
    cg = 0x36 + s*1;
    cb = 0x90 + s*2;

    if(parts[i].type==PT_ACRN){
        cr = 115;
        cg = 250;
        cb = 187;
    }

    if (cr>=255)
        cr = 255;
    if (cg>=255)
        cg = 255;
    if (cb>=255)
        cb = 255;

    blendpixel(vid, nx, ny, cr, cg, cb, 255);

    if (cmode==CM_BLOB)
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
