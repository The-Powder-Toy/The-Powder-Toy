#include "defines.h"
#include "air.h"
#include "powder.h"
#include "graphics.h"
#include "font.h"
#include <SDL/SDL.h>

pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f)
{
    int i,j,x,y,w,h,r,g,b,c;
    pixel p, *q;
    w = (sw+f-1)/f;
    h = (sh+f-1)/f;
    q = malloc(w*h*PIXELSIZE);
    for(y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            r = g = b = c = 0;
            for(j=0; j<f; j++)
                for(i=0; i<f; i++)
                    if(x*f+i<sw && y*f+j<sh)
                    {
                        p = src[(y*f+j)*sw + (x*f+i)];
                        if(p)
                        {
                            r += PIXR(p);
                            g += PIXG(p);
                            b += PIXB(p);
                            c ++;
                        }
                    }
            if(c>1)
            {
                r = (r+c/2)/c;
                g = (g+c/2)/c;
                b = (b+c/2)/c;
            }
            q[y*w+x] = PIXRGB(r, g, b);
        }
    *qw = w;
    *qh = h;
    return q;
}

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch)
{
    pixel *dst;
    int j;
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
    for(j=0; j<h; j++)
    {
        memcpy(dst, src, w*PIXELSIZE);
        dst+=sdl_scrn->pitch/PIXELSIZE;
        src+=pitch;
    }
    if(SDL_MUSTLOCK(sdl_scrn))
        SDL_UnlockSurface(sdl_scrn);
    SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit_2(int x, int y, int w, int h, pixel *src, int pitch)
{
    pixel *dst;
    int j;
    int i,k;
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
    for(j=0; j<h; j++)
    {
        for(k=0; k<sdl_scale; k++)
        {
            for(i=0; i<w; i++)
            {
                dst[i*2]=src[i];
                dst[i*2+1]=src[i];
            }
            dst+=sdl_scrn->pitch/PIXELSIZE;
        }
        src+=pitch;
    }
    if(SDL_MUSTLOCK(sdl_scrn))
        SDL_UnlockSurface(sdl_scrn);
    SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void sdl_blit(int x, int y, int w, int h, pixel *src, int pitch)
{
    if(sdl_scale == 2)
        sdl_blit_2(x, y, w, h, src, pitch);
    else
        sdl_blit_1(x, y, w, h, src, pitch);
}

void drawblob(pixel *vid, int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
    blendpixel(vid, x+1, y, cr, cg, cb, 112);
    blendpixel(vid, x-1, y, cr, cg, cb, 112);
    blendpixel(vid, x, y+1, cr, cg, cb, 112);
    blendpixel(vid, x, y-1, cr, cg, cb, 112);

    blendpixel(vid, x+1, y-1, cr, cg, cb, 64);
    blendpixel(vid, x-1, y-1, cr, cg, cb, 64);
    blendpixel(vid, x+1, y+1, cr, cg, cb, 64);
    blendpixel(vid, x-1, y+1, cr, cg, cb, 64);
}

void draw_tool(pixel *vid_buf, int b, int sl, int sr, unsigned pc, unsigned iswall)
{
    int x, y, i, j, c;
    int bo = b;
    if(iswall==1)
    {
        b = b-100;
        x = (2+32*((b-22)/1));
        y = YRES+2+40;
        switch(b)
        {
        case WL_WALLELEC:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                    else
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = PIXPACK(0x808080);
                    }
                }
            }
            break;
        case 23:
            for(j=1; j<15; j++)
            {
                for(i=1; i<6+j; i++)
                {
                    if(!(i&j&1))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
                for(; i<27; i++)
                {
                    if(i&j&1)
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 24:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 25:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
                    drawtext(vid_buf, x+4, y+3, "\x8D", 255, 255, 255, 255);
                }
            }
            for(i=9; i<27; i++)
            {
                drawpixel(vid_buf, x+i, y+8+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
            }
            break;
        case 26:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
                }
            }
            drawtext(vid_buf, x+9, y+3, "\xA1", 32, 64, 128, 255);
            drawtext(vid_buf, x+9, y+3, "\xA0", 255, 255, 255, 255);
            break;
        case 27:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 28:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 29:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 30:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<13; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            for(j=1; j<15; j++)
            {
                for(i=14; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 32:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 33:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 34:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 36:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("AIR")/2, y+4, "AIR", c, c, c, 255);
            break;
        case 37:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("HEAT")/2, y+4, "HEAT", c, c, c, 255);
            break;
        case 38:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("COOL")/2, y+4, "COOL", c, c, c, 255);
            break;
        case 39:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("VAC")/2, y+4, "VAC", c, c, c, 255);
            break;
        default:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
        }
        if(b==30)
        {
            for(j=4; j<12; j++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
            }
        }
    }
    else
    {
        x = 2+32*(b/2);
        y = YRES+2+20*(b%2);
        for(j=1; j<15; j++)
        {
            for(i=1; i<27; i++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
            }
        }
        if(b==0)
        {
            for(j=4; j<12; j++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
            }
        }
        c = PIXB(ptypes[b].pcolors) + 3*PIXG(ptypes[b].pcolors) + 2*PIXR(ptypes[b].pcolors);
        if(c<544)
        {
            c = 255;
        }
        else
        {
            c = 0;
        }
        drawtext(vid_buf, x+14-textwidth((char *)ptypes[b].name)/2, y+4, (char *)ptypes[b].name, c, c, c, 255);
    }
    if(bo==sl || bo==sr)
    {
        c = 0;
        if(bo==sl)
            c |= PIXPACK(0xFF0000);
        if(bo==sr)
            c |= PIXPACK(0x0000FF);
        for(i=0; i<30; i++)
        {
            vid_buf[(XRES+BARSIZE)*(y-1)+(x+i-1)] = c;
            vid_buf[(XRES+BARSIZE)*(y+16)+(x+i-1)] = c;
        }
        for(j=0; j<18; j++)
        {
            vid_buf[(XRES+BARSIZE)*(y+j-1)+(x-1)] = c;
            vid_buf[(XRES+BARSIZE)*(y+j-1)+(x+28)] = c;
        }
    }
}

int draw_tool_xy(pixel *vid_buf, int x, int y, int b, unsigned pc)
{
    int i, j, c;
    if(b>=121)
    {
        b = b-100;
        //x = (2+32*((b-22)/1));
        //y = YRES+2+40;
        switch(b)
        {
        case WL_WALLELEC:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                    else
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = PIXPACK(0x808080);
                    }
                }
            }
            break;
        case 23:
            for(j=1; j<15; j++)
            {
                for(i=1; i<6+j; i++)
                {
                    if(!(i&j&1))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
                for(; i<27; i++)
                {
                    if(i&j&1)
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 24:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 25:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
                    drawtext(vid_buf, x+4, y+3, "\x8D", 255, 255, 255, 255);
                }
            }
            for(i=9; i<27; i++)
            {
                drawpixel(vid_buf, x+i, y+8+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
            }
            break;
        case 26:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
                }
            }
            drawtext(vid_buf, x+9, y+3, "\xA1", 32, 64, 128, 255);
            drawtext(vid_buf, x+9, y+3, "\xA0", 255, 255, 255, 255);
            break;
        case 27:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 28:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 29:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 30:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<13; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            for(j=1; j<15; j++)
            {
                for(i=14; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 32:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 33:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        case 34:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    if(!(i%2) && !(j%2))
                    {
                        vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                    }
                }
            }
            break;
        case 36:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("AIR")/2, y+4, "AIR", c, c, c, 255);
            break;
        case 37:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("HEAT")/2, y+4, "HEAT", c, c, c, 255);
            break;
        case 38:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("COOL")/2, y+4, "COOL", c, c, c, 255);
            break;
        case 39:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            drawtext(vid_buf, x+14-textwidth("VAC")/2, y+4, "VAC", c, c, c, 255);
            break;
        case 40:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
            break;
        default:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
                }
            }
        }
        if(b==30)
        {
            for(j=4; j<12; j++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
            }
        }
    }
    else
    {
        //x = 2+32*(b/2);
        //y = YRES+2+20*(b%2);
        for(j=1; j<15; j++)
        {
            for(i=1; i<27; i++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
            }
        }
        if(b==0)
        {
            for(j=4; j<12; j++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
                vid_buf[(XRES+BARSIZE)*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
            }
        }
        c = PIXB(ptypes[b].pcolors) + 3*PIXG(ptypes[b].pcolors) + 2*PIXR(ptypes[b].pcolors);
        if(c<544)
        {
            c = 255;
        }
        else
        {
            c = 0;
        }
        drawtext(vid_buf, x+14-textwidth((char *)ptypes[b].name)/2, y+4, (char *)ptypes[b].name, c, c, c, 255);
    }
    return 26;
}

void draw_menu(pixel *vid_buf, int i, int hover)
{

    //drawtext(vid_buf, XRES+1, /*(12*i)+2*/((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2), msections[i].icon, 255, 255, 255, 255);
#ifdef MENUV3
    drawrect(vid_buf, XRES-2, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
    if(hover==i)
    {
        fillrect(vid_buf, XRES-2, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
        drawtext(vid_buf, XRES+1, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 0, 0, 0, 255);
    }
    else
    {
        drawtext(vid_buf, XRES+1, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 255, 255, 255, 255);
    }
#else
    drawtext(vid_buf, XRES+1, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 255, 255, 255, 255);
#endif
}

#ifdef WIN32
_inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#else
inline void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#endif
{
    pixel t;
    if(x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
        return;
    if(a!=255)
    {
        t = vid[y*(XRES+BARSIZE)+x];
        r = (a*r + (255-a)*PIXR(t)) >> 8;
        g = (a*g + (255-a)*PIXG(t)) >> 8;
        b = (a*b + (255-a)*PIXB(t)) >> 8;
    }
    vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}

#ifdef WIN32
_inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
#else
inline int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
#endif
{
    int i, j, w, bn = 0, ba = 0;
    char *rp = font_data + font_ptrs[c];
    w = *(rp++);
    for(j=0; j<FONT_H; j++)
        for(i=0; i<w; i++)
        {
            if(!bn)
            {
                ba = *(rp++);
                bn = 8;
            }
            drawpixel(vid, x+i, y+j, r, g, b, ((ba&3)*a)/3);
            ba >>= 2;
            bn -= 2;
        }
    return x + w;
}

int drawtext(pixel *vid, int x, int y, char *s, int r, int g, int b, int a)
{
    int sx = x;
    for(; *s; s++)
    {
        if(*s == '\n')
        {
            x = sx;
            y += FONT_H+2;
        }
        else if(*s == '\b')
        {
            switch(s[1])
            {
            case 'w':
                r = g = b = 255;
                break;
            case 'g':
                r = g = b = 192;
                break;
            case 'o':
                r = 255;
                g = 216;
                b = 32;
                break;
            case 'r':
                r = 255;
                g = b = 0;
                break;
            case 'b':
                r = g = 0;
                b = 255;
                break;
            }
            s++;
        }
        else
            x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
    }
    return x;
}

void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
    int i;
    for(i=0; i<=w; i++)
    {
        drawpixel(vid, x+i, y, r, g, b, a);
        drawpixel(vid, x+i, y+h, r, g, b, a);
    }
    for(i=1; i<h; i++)
    {
        drawpixel(vid, x, y+i, r, g, b, a);
        drawpixel(vid, x+w, y+i, r, g, b, a);
    }
}

void fillrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
    int i,j;
    for(j=1; j<h; j++)
        for(i=1; i<w; i++)
            drawpixel(vid, x+i, y+j, r, g, b, a);
}

void clearrect(pixel *vid, int x, int y, int w, int h)
{
    int i;
    for(i=1; i<h; i++)
        memset(vid+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
}

void drawdots(pixel *vid, int x, int y, int h, int r, int g, int b, int a)
{
    int i;
    for(i=0; i<=h; i+=2)
        drawpixel(vid, x, y+i, r, g, b, a);
}

int textwidth(char *s)
{
    int x = 0;
    for(; *s; s++)
        x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
    return x-1;
}

int drawtextmax(pixel *vid, int x, int y, int w, char *s, int r, int g, int b, int a)
{
    int i;
    w += x-5;
    for(; *s; s++)
    {
        if(x+font_data[font_ptrs[(int)(*(unsigned char *)s)]]>=w && x+textwidth(s)>=w+5)
            break;
        x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
    }
    if(*s)
        for(i=0; i<3; i++)
            x = drawchar(vid, x, y, '.', r, g, b, a);
    return x;
}

int textnwidth(char *s, int n)
{
    int x = 0;
    for(; *s; s++)
    {
        if(!n)
            break;
        x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
        n--;
    }
    return x-1;
}

int textwidthx(char *s, int w)
{
    int x=0,n=0,cw;
    for(; *s; s++)
    {
        cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
        if(x+(cw/2) >= w)
            break;
        x += cw;
        n++;
    }
    return n;
}

#ifdef WIN32
_inline void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#else
inline void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
#endif
{
    pixel t;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    if(a!=255)
    {
        t = vid[y*(XRES+BARSIZE)+x];
        r = (a*r + (255-a)*PIXR(t)) >> 8;
        g = (a*g + (255-a)*PIXG(t)) >> 8;
        b = (a*b + (255-a)*PIXB(t)) >> 8;
    }
    vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}

void draw_icon(pixel *vid_buf, int x, int y, char ch, int flag)
{
    char t[2];
    t[0] = ch;
    t[1] = 0;
    if(flag)
    {
        fillrect(vid_buf, x-1, y-1, 17, 17, 255, 255, 255, 255);
        drawtext(vid_buf, x+3, y+2, t, 0, 0, 0, 255);
    }
    else
    {
        drawrect(vid_buf, x, y, 15, 15, 255, 255, 255, 255);
        drawtext(vid_buf, x+3, y+2, t, 255, 255, 255, 255);
    }
}

void draw_air(pixel *vid)
{
    int x, y, i, j;
    pixel c;

    if(cmode == 2)
        return;

    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
        {
            if(cmode)
            {
                if(pv[y][x] > 0.0f)
                    c  = PIXRGB(clamp_flt(pv[y][x], 0.0f, 8.0f), 0, 0);
                else
                    c  = PIXRGB(0, 0, clamp_flt(-pv[y][x], 0.0f, 8.0f));
            }
            else
                c  = PIXRGB(clamp_flt(fabsf(vx[y][x]), 0.0f, 8.0f),
                            clamp_flt(pv[y][x], 0.0f, 8.0f),
                            clamp_flt(fabsf(vy[y][x]), 0.0f, 8.0f));
            for(j=0; j<CELL; j++)
                for(i=0; i<CELL; i++)
                    vid[(x*CELL+i) + (y*CELL+j)*(XRES+BARSIZE)] = c;
        }
}

void draw_line(pixel *vid, int x1, int y1, int x2, int y2, int r, int g, int b, int a)  //Draws a line
{
    int dx, dy, i, sx, sy, check, e, x, y;

    dx = abs(x1-x2);
    dy = abs(y1-y2);
    sx = sign(x2-x1);
    sy = sign(y2-y1);
    x = x1;
    y = y1;
    check = 0;

    if (dy>dx)
    {
        dx = dx+dy;
        dy = dx-dy;
        dx = dx-dy;
        check = 1;
    }

    e = (dy<<2)-dx;
    for (i=0; i<=dx; i++)
    {
        vid[x+y*a] =PIXRGB(r, g, b);
        if (e>=0)
        {
            if (check==1)
                x = x+sx;
            else
                y = y+sy;
            e = e-(dx<<2);
        }
        if (check==1)
            y = y+sy;
        else
            x = x+sx;
        e = e+(dy<<2);
    }
}

void addpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
{
    pixel t;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    t = vid[y*(XRES+BARSIZE)+x];
    r = (a*r + 255*PIXR(t)) >> 8;
    g = (a*g + 255*PIXG(t)) >> 8;
    b = (a*b + 255*PIXB(t)) >> 8;
    if(r>255)
        r = 255;
    if(g>255)
        g = 255;
    if(b>255)
        b = 255;
    vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}

void xor_pixel(int x, int y, pixel *vid)
{
    int c;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    c = vid[y*(XRES+BARSIZE)+x];
    c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
    if(c<512)
        vid[y*(XRES+BARSIZE)+x] = PIXPACK(0xC0C0C0);
    else
        vid[y*(XRES+BARSIZE)+x] = PIXPACK(0x404040);
}

void xor_line(int x1, int y1, int x2, int y2, pixel *vid)
{
    int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
    float e, de;
    if(cp)
    {
        y = x1;
        x1 = y1;
        y1 = y;
        y = x2;
        x2 = y2;
        y2 = y;
    }
    if(x1 > x2)
    {
        y = x1;
        x1 = x2;
        x2 = y;
        y = y1;
        y1 = y2;
        y2 = y;
    }
    dx = x2 - x1;
    dy = abs(y2 - y1);
    e = 0.0f;
    if(dx)
        de = dy/(float)dx;
    else
        de = 0.0f;
    y = y1;
    sy = (y1<y2) ? 1 : -1;
    for(x=x1; x<=x2; x++)
    {
        if(cp)
            xor_pixel(y, x, vid);
        else
            xor_pixel(x, y, vid);
        e += de;
        if(e >= 0.5f)
        {
            y += sy;
            e -= 1.0f;
        }
    }
}

void xor_rect(pixel *vid, int x, int y, int w, int h)
{
    int i;
    for(i=0; i<w; i+=2)
    {
        xor_pixel(x+i, y, vid);
        xor_pixel(x+i, y+h-1, vid);
    }
    for(i=2; i<h; i+=2)
    {
        xor_pixel(x, y+i, vid);
        xor_pixel(x+w-1, y+i, vid);
    }
}