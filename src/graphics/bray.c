#include <element.h>

int graphics_BRAY(GRAPHIC_FUNC_ARGS)
{
    int x,y,cr,cg,cb,t;
    t = parts[i].type;
    if (parts[i].tmp==0)
    {

        int trans = parts[i].life * 7;
        if (trans>255) trans = 255;
        if (parts[i].ctype)
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
            blendpixel(vid, nx, ny, cr, cg, cb, trans);
        }
        else
            blendpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), trans);
    }
    else if (parts[i].tmp==1)
    {
        int trans = parts[i].life/4;
        if (trans>255) trans = 255;
        if (parts[i].ctype)
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
            blendpixel(vid, nx, ny, cr, cg, cb, trans);
        }
        else
            blendpixel(vid, nx, ny, PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors), trans);
    }
    else if (parts[i].tmp==2)
    {
        int trans = parts[i].life*100;
        if (trans>255) trans = 255;
        blendpixel(vid, nx, ny, 255, 150, 50, trans);
    }
    return 0;
}
