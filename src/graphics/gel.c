#include <element.h>

int graphics_GEL(GRAPHIC_FUNC_ARGS)
{
int fg,fb,fr,x,y;
    fg = 242;
    fb = 170;
    fr = 199;
    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB((int)restrict_flt(0x44 + fr, 0, 255), (int)restrict_flt(0x88 + fg*8, 0, 255), (int)restrict_flt(0x44 + fb*8, 0, 255));
    if(cmode == CM_FIRE||cmode==CM_BLOB || cmode==CM_FANCY || cmode==CM_AWESOME || cmode==CM_PREAWE)
    {
        x = nx/CELL;
        y = ny/CELL;
        //fg += fire_g[y][x];
        if(fg > 255) fg = 255;
        fire_g[y][x] = fg;
        //fb += fire_b[y][x];
        if(fb > 255) fb = 255;
        fire_b[y][x] = fb;
        //fr += fire_r[y][x];
        if(fr > 255) fr = 255;
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
