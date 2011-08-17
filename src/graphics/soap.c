#include <element.h>

int graphics_SOAP(GRAPHIC_FUNC_ARGS)
{
    int cr,cg,cb,t,x,y;
    t = parts[i].type;
    if ((parts[i].ctype&7) == 7)
        draw_line(vid, nx, ny, (int)(parts[parts[i].tmp].x+0.5f), (int)(parts[parts[i].tmp].y+0.5f), 245, 245, 220, XRES+BARSIZE);
    blendpixel(vid,nx,ny,PIXR(ptypes[t].pcolors),PIXG(ptypes[t].pcolors),PIXB(ptypes[t].pcolors),255);
    return 1;
}
