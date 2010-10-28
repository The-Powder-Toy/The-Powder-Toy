#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>

#ifdef OpenGL
#ifdef MACOSX
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#endif

#include <defines.h>
#include <air.h>
#include <powder.h>
#include <graphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>


unsigned cmode = 3;
SDL_Surface *sdl_scrn;
int sdl_scale = 1;

unsigned char fire_r[YRES/CELL][XRES/CELL];
unsigned char fire_g[YRES/CELL][XRES/CELL];
unsigned char fire_b[YRES/CELL][XRES/CELL];

unsigned int fire_alpha[CELL*3][CELL*3];
pixel *fire_bg;

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
#ifdef OpenGL
    RenderScene();
#else
    if(sdl_scale == 2)
        sdl_blit_2(x, y, w, h, src, pitch);
    else
        sdl_blit_1(x, y, w, h, src, pitch);
#endif
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
#ifdef OpenGL
        fillrect(vid_buf, x, y, 28, 16, PIXR(pc), PIXG(pc), PIXB(pc), 255);
#else
        for(j=1; j<15; j++)
        {
            for(i=1; i<27; i++)
            {
                vid_buf[(XRES+BARSIZE)*(y+j)+(x+i)] = pc;
            }
        }
#endif
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
    drawrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
    if(hover==i)
    {
        fillrect(vid_buf, (XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
        drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 0, 0, 0, 255);
    }
    else
    {
        drawtext(vid_buf, (XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 255, 255, 255, 255);
    }
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

int drawtext(pixel *vid, int x, int y, const char *s, int r, int g, int b, int a)
{
#ifdef OpenGL
#else
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
#endif
    return x;
}
int drawtextwrap(pixel *vid, int x, int y, int w, const char *s, int r, int g, int b, int a)
{
#ifdef OpenGL
#else
    int sx = x;
    int rh = 12;
    int rw = 0;
    int cw = x;
    for(; *s; s++)
    {
        if(*s == '\n')
        {
            x = sx;
            rw = 0;
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
        {
            if(x-cw>=w) {
                x = sx;
                rw = 0;
                y+=FONT_H+2;
                rh+=FONT_H+2;
            }
            x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
        }
    }
#endif
    return rh;
}

void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
    glBegin(GL_LINE_LOOP);
    glColor4ub(r, g, b, a);
    glVertex2i(x, y);
    glVertex2i(x+w, y);
    glVertex2i(x+w, y+h);
    glVertex2i(x, y+h);
    glEnd();
#else
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
#endif
}

void fillrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
    glBegin(GL_QUADS);
    glColor4ub(r, g, b, a);
    glVertex2i(x, y);
    glVertex2i(x+w, y);
    glVertex2i(x+w, y+h);
    glVertex2i(x, y+h);
    glEnd();
#else
    int i,j;
    for(j=1; j<h; j++)
        for(i=1; i<w; i++)
            drawpixel(vid, x+i, y+j, r, g, b, a);
#endif // OpenGL
}

void clearrect(pixel *vid, int x, int y, int w, int h)
{
    int i;
    for(i=1; i<h; i++)
        memset(vid+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
}
void drawdots(pixel *vid, int x, int y, int h, int r, int g, int b, int a)
{
#ifdef OpenGL
    int i;
    glBegin(GL_QUADS);
    glColor4ub(r, g, b, a);
    for(i = 0; i <= h; i +=2)
        glVertex2i(x, y+i);
    glEnd();
#else
    int i;
    for(i=0; i<=h; i+=2)
        drawpixel(vid, x, y+i, r, g, b, a);
#endif //OpenGL
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
int textnheight(char *s, int n, int w)
{
    int x = 0;
	//TODO: Implement Textnheight for wrapped text
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
#ifdef OpenGL
    if(x<0 || y<0 || x>=XRES || r>=YRES)
        return;
    if(a!=255)
    {
        glBegin (GL_QUADS);
        glColor4ub(r,g,b,a);
        glVertex2i(x, y);
    }
    vid[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
#else
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
#endif //OpenGL
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
    sx = isign(x2-x1);
    sy = isign(y2-y1);
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

void draw_parts(pixel *vid)
{
    int i, x, y, t, nx, ny, r, s;
    int cr, cg, cb;
    float fr, fg, fb;
    float pt = R_TEMP;
    for(i = 0; i<NPART; i++) {
#ifdef OpenGL
        if(cmode == 6) //If fire mode
        {

            if(t==PT_MWAX)
            {
                for(x=-1; x<=1; x++)
                {
                    for(y=-1; y<=1; y++)
                    {
                        if ((abs(x) == 0) && (abs(y) == 0))
                            blendpixel(vid,x+nx,y+ny,224,224,170,255);
                        else if (abs(y) != 0 && abs(x) != 0)
                            blendpixel(vid,x+nx,y+ny,224,224,170,20);
                        else
                            blendpixel(vid,x+nx,y+ny,224,224,170,40);
                    }
                }

            }

            else if(t==PT_PLUT)
            {
                int tempx;
                int tempy;
                cr = 0x40;
                cg = 0x70;
                cb = 0x20;
                blendpixel(vid, nx, ny, cr, cg, cb, 192);
                blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
                blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
                blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
                blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
                for(tempx = 2; tempx < 10; tempx++) {
                    for(tempy = 2; tempy < 10; tempy++) {
                        blendpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
                        blendpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
                        blendpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
                        blendpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
                    }
                }
            }
        }
        if(parts[i].type) {
            //Do nothing
            t = parts[i].type;
            nx = (int)(parts[i].x+0.5f);
            ny = (int)(parts[i].y+0.5f);
            glBegin (GL_POINTS);
            glColor3ub (PIXR(ptypes[t].pcolors), PIXG(ptypes[t].pcolors), PIXB(ptypes[t].pcolors));
            glVertex2d (nx, ny);
            glEnd ();
        }
#else
        if(parts[i].type) {
            t = parts[i].type;

            nx = (int)(parts[i].x+0.5f);
            ny = (int)(parts[i].y+0.5f);

            if(cmode!=CM_HEAT)
            {
                if(t==PT_STKM)  //Just draw head here
                {
                    char buff[10];  //Buffer for HP

                    if(mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3))  //If mous is in the head
                    {
                        sprintf(buff, "%3d", parts[i].life);  //Show HP
                        drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
                    }

                    for(r=-2; r<=1; r++)  //Here I use r variable not as I should, but I think you will excuse me :-p
                    {
                        s = XRES+BARSIZE;
                        vid[(ny-2)*s+nx+r] = ptypes[(int)player[2]].pcolors;
                        vid[(ny+2)*s+nx+r+1] = ptypes[(int)player[2]].pcolors;
                        vid[(ny+r+1)*s+nx-2] = ptypes[(int)player[2]].pcolors;
                        vid[(ny+r)*s+nx+2] = ptypes[(int)player[2]].pcolors;
                    }
                    draw_line(vid , nx, ny+3, player[3], player[4], 255, 255, 255, s);
                    draw_line(vid , player[3], player[4], player[7], player[8], 255, 255, 255, s);
                    draw_line(vid , nx, ny+3, player[11], player[12], 255, 255, 255, s);
                    draw_line(vid , player[11], player[12], player[15], player[16], 255, 255, 255, s);

                    isplayer = 1;  //It's a secret. Tssss...
                }
                if(t==PT_MWAX&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,224,224,170,255);
                            else if (abs(y) != 0 && abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,224,224,170,20);
                            else
                                blendpixel(vid,x+nx,y+ny,224,224,170,40);
                        }
                    }

                }
                else if(t==PT_ACID)
                {
                    if(parts[i].life>255) parts[i].life = 255;
                    if(parts[i].life<47) parts[i].life = 48;
                    s = (255/((parts[i].life-46)*28));
                    if(s==0) s = 1;
                    cr = PIXR(ptypes[t].pcolors)/s;
                    cg = PIXG(ptypes[t].pcolors)/s;
                    cb = PIXB(ptypes[t].pcolors)/s;
                    if(cmode==6) {
                        for(x=-1; x<=1; x++)
                        {
                            for(y=-1; y<=1; y++)
                            {
                                if ((abs(x) == 0) && (abs(y) == 0))
                                    blendpixel(vid,x+nx,y+ny,cr,cg,cb,100);
                                else if (abs(y) != 0 || abs(x) != 0)
                                    blendpixel(vid,x+nx,y+ny,cr,cg,cb,40);
                            }
                        }
                    } else {
                        blendpixel(vid, nx, ny, cr, cg, cb, 255);
                    }

                    if(cmode==4)
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
                }
                else if(t==PT_OIL&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,64,64,16,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,64,64,16,40);
                        }
                    }
                }
                else if(t==PT_NEUT)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
                        cg = 8;
                        cb = 12;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                    }
                    else
                    {
                        cr = 0x20;
                        cg = 0xE0;
                        cb = 0xFF;
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
                } else if(t==PT_PLUT&&cmode == 6)
                {
                    int tempx;
                    int tempy;
                    cr = 0x40;
                    cg = 0x70;
                    cb = 0x20;
                    blendpixel(vid, nx, ny, cr, cg, cb, 192);
                    blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
                    blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
                    blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
                    blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
                    for(tempx = 2; tempx < 10; tempx++) {
                        for(tempy = 2; tempy < 10; tempy++) {
                            blendpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
                        }
                    }
                } else if(t==PT_URAN&&cmode == 6)
                {
                    int tempx;
                    int tempy;
                    cr = 0x70;
                    cg = 0x70;
                    cb = 0x20;
                    blendpixel(vid, nx, ny, cr, cg, cb, 192);
                    blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
                    blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
                    blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
                    blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
                    for(tempx = 2; tempx < 10; tempx++) {
                        for(tempy = 2; tempy < 10; tempy++) {
                            blendpixel(vid, nx+tempx, ny-tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx-tempx, ny+tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx+tempx, ny+tempy, cr, cg, cb, 5);
                            blendpixel(vid, nx-tempx, ny-tempy, cr, cg, cb, 5);
                        }
                    }
                } else if(t==PT_SLTW&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,64,80,240,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,64,80,240,50);
                        }
                    }
                }
                else if(t==PT_PHOT)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cg = 0;
                        cb = 0;
                        cr = 0;
                        for(x=0; x<12; x++) {
                            cr += (parts[i].ctype >> (x+18)) & 1;
                            cb += (parts[i].ctype >>  x)     & 1;
                        }
                        for(x=0; x<14; x++)
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
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                    }
                    else
                    {
                        cg = 0;
                        cb = 0;
                        cr = 0;
                        for(x=0; x<12; x++) {
                            cr += (parts[i].ctype >> (x+18)) & 1;
                            cb += (parts[i].ctype >>  x)     & 1;
                        }
                        for(x=0; x<14; x++)
                            cg += (parts[i].ctype >> (x+9))  & 1;
                        x = 624/(cr+cg+cb+1);
                        cr *= x;
                        cg *= x;
                        cb *= x;
                        cr = cr>255?255:cr;
                        cg = cg>255?255:cg;
                        cb = cb>255?255:cb;
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
                else if(t==PT_SWCH && parts[i].life == 10)
                {
                    x = nx;
                    y = ny;
                    blendpixel(vid,x,y,17,217,24,255);
                }
                else if(t==PT_LNTG&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,128,160,223,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,128,160,223,50);
                        }
                    }
                }
                else if(t==PT_SMKE)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        x = nx/CELL;
                        y = ny/CELL;
                        cg = 10;
                        cb = 10;
                        cr = 10;
                        cg += fire_g[y][x];
                        if(cg > 50) cg = 50;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 50) cb = 50;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 50) cr = 50;
                        fire_r[y][x] = cr;
                    }
                    else
                    {
                        for(x=-3; x<4; x++)
                        {
                            for(y=-3; y<4; y++)
                            {
                                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                                    blendpixel(vid,x+nx,y+ny,100,100,100,30);
                                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                                    blendpixel(vid,x+nx,y+ny,100,100,100,10);
                                if (abs(x)+abs(y) == 2)
                                    blendpixel(vid,x+nx,y+ny,100,100,100,20);
                            }
                        }
                    }
                }
                else if(t==PT_WATR&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,32,48,208,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,32,48,208,50);
                        }
                    }

                } else if(t==PT_DSTW&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,32,48,208,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,32,48,208,50);
                        }
                    }
                }
                else if(t==PT_NITR&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,32,224,16,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,32,224,16,50);
                        }
                    }

                }
                else if(t==PT_LRBD&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,170,170,170,100);
                            else if (abs(y) != 0 || abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,170,170,170,50);
                        }
                    }

                }

                else if(t==PT_NBLE&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,235,73,23,100);
                            else if (abs(y) != 0 && abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,235,73,23,30);
                            else
                                blendpixel(vid,x+nx,y+ny,235,73,23,50);
                        }
                    }

                }
                else if(t==PT_GAS&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if ((abs(x) == 0) && (abs(y) == 0))
                                blendpixel(vid,x+nx,y+ny,255,255,0,180);
                            else if (abs(y) != 0 && abs(x) != 0)
                                blendpixel(vid,x+nx,y+ny,255,255,0,50);
                            else
                                blendpixel(vid,x+nx,y+ny,255,255,0,80);
                        }
                    }

                }
                else if(t==PT_WTRV)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        x = nx/CELL;
                        y = ny/CELL;
                        cg = PIXG(ptypes[t].pcolors)/3;
                        cb = PIXB(ptypes[t].pcolors)/3;
                        cr = PIXR(ptypes[t].pcolors)/3;
                        cg += fire_g[y][x];
                        if(cg > PIXG(ptypes[t].pcolors)/2) cg = PIXG(ptypes[t].pcolors)/2;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > PIXB(ptypes[t].pcolors)/2) cb = PIXB(ptypes[t].pcolors)/2;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > PIXR(ptypes[t].pcolors)/2) cr = PIXR(ptypes[t].pcolors)/2;
                        fire_r[y][x] = cr;
                    }
                    else
                    {
                        for(x=-3; x<4; x++)
                        {
                            for(y=-3; y<4; y++)
                            {
                                if (abs(x)+abs(y) <2 && !(abs(x)==2||abs(y)==2))
                                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
                                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
                                if (abs(x)+abs(y) == 2)
                                    blendpixel(vid,x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
                            }
                        }
                    }
                }
                else if(t==PT_THDR)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
                        cg = 16;
                        cb = 20;
                        cr = 12;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                    }
                    else
                    {
                        cr = 0xFF;
                        cg = 0xFF;
                        cb = 0xA0;
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
                else if(t==PT_GLOW)
                {
                    fg = 0;
                    fb = 0;
                    fr = 0;
                    if(pv[ny/CELL][nx/CELL]>0) {
                        fg = 6 * pv[ny/CELL][nx/CELL];
                        fb = 4 * pv[ny/CELL][nx/CELL];
                        fr = 2 * pv[ny/CELL][nx/CELL];
                    }
                    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB((int)restrict_flt(0x44 + fr*8, 0, 255), (int)restrict_flt(0x88 + fg*8, 0, 255), (int)restrict_flt(0x44 + fb*8, 0, 255));
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        x = nx/CELL;
                        y = ny/CELL;
                        fg += fire_g[y][x];
                        if(fg > 255) fg = 255;
                        fire_g[y][x] = fg;
                        fb += fire_b[y][x];
                        if(fb > 255) fb = 255;
                        fire_b[y][x] = fb;
                        fr += fire_r[y][x];
                        if(fr > 255) fr = 255;
                        fire_r[y][x] = fr;
                    }
                    if(cmode == 4) {
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
                }
                else if(t==PT_LCRY)
                {
                    uint8 GR = 0x50+(parts[i].life*10);
                    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, GR, GR);
                    if(cmode == 4) {
                        blendpixel(vid, nx+1, ny, GR, GR, GR, 223);
                        blendpixel(vid, nx-1, ny, GR, GR, GR, 223);
                        blendpixel(vid, nx, ny+1, GR, GR, GR, 223);
                        blendpixel(vid, nx, ny-1, GR, GR, GR, 223);

                        blendpixel(vid, nx+1, ny-1, GR, GR, GR, 112);
                        blendpixel(vid, nx-1, ny-1, GR, GR, GR, 112);
                        blendpixel(vid, nx+1, ny+1, GR, GR, GR, 112);
                        blendpixel(vid, nx-1, ny+1, GR, GR, GR, 112);
                    }
                }
                else if(t==PT_PCLN)
                {
                    uint8 GR = 0x3B+(parts[i].life*19);
                    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, GR, 10);
                    if(cmode == 4) {
                        blendpixel(vid, nx+1, ny, GR, GR, 10, 223);
                        blendpixel(vid, nx-1, ny, GR, GR, 10, 223);
                        blendpixel(vid, nx, ny+1, GR, GR, 10, 223);
                        blendpixel(vid, nx, ny-1, GR, GR, 10, 223);

                        blendpixel(vid, nx+1, ny-1, GR, GR, 10, 112);
                        blendpixel(vid, nx-1, ny-1, GR, GR, 10, 112);
                        blendpixel(vid, nx+1, ny+1, GR, GR, 10, 112);
                        blendpixel(vid, nx-1, ny+1, GR, GR, 10, 112);
                    }
                }
                else if(t==PT_HSWC)
                {
                    uint8 GR = 0x3B+(parts[i].life*19);
                    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(GR, 10, 10);
                    if(cmode == 4) {
                        blendpixel(vid, nx+1, ny, GR, 10, 10, 223);
                        blendpixel(vid, nx-1, ny, GR, 10, 10, 223);
                        blendpixel(vid, nx, ny+1, GR, 10, 10, 223);
                        blendpixel(vid, nx, ny-1, GR, 10, 10, 223);

                        blendpixel(vid, nx+1, ny-1, GR, 10, 10, 112);
                        blendpixel(vid, nx-1, ny-1, GR, 10, 10, 112);
                        blendpixel(vid, nx+1, ny+1, GR, 10, 10, 112);
                        blendpixel(vid, nx-1, ny+1, GR, 10, 10, 112);
                    }
                }
                else if(t==PT_PLSM)
                {
                    float ttemp = (float)parts[i].life;
                    int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
                    uint8 R = plasma_data[caddress];
                    uint8 G = plasma_data[caddress+1];
                    uint8 B = plasma_data[caddress+2];
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr = R/8;
                        cg = G/8;
                        cb = B/8;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
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
                } else if(t==PT_HFLM)
                {
                    float ttemp = (float)parts[i].life;
                    int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
                    uint8 R = hflm_data[caddress];
                    uint8 G = hflm_data[caddress+1];
                    uint8 B = hflm_data[caddress+2];
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr = R/8;
                        cg = G/8;
                        cb = B/8;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
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
                } else if(t==PT_FIRW&&parts[i].tmp>=3)
                {
                    float ttemp = (float)parts[i].tmp-4;
                    int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
                    uint8 R = firw_data[caddress];
                    uint8 G = firw_data[caddress+1];
                    uint8 B = firw_data[caddress+2];
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr = R/2;
                        cg = G/2;
                        cb = B/2;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
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
                }
                else if(t==PT_FIRE && parts[i].life)
                {
                    float ttemp = (float)((int)(parts[i].life/2));
                    int caddress = restrict_flt(restrict_flt(ttemp, 0.0f, 200.0f)*3, 0.0f, (200.0f*3)-3);
                    uint8 R = flm_data[caddress];
                    uint8 G = flm_data[caddress+1];
                    uint8 B = flm_data[caddress+2];
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr = R/8;
                        cg = G/8;
                        cb = B/8;
                        x = nx/CELL;
                        y = ny/CELL;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                    }
                    else
                    {
                        cr = parts[i].life * 8;
                        cg = parts[i].life * 2;
                        cb = parts[i].life;
                        if(cr>255) cr = 255;
                        if(cg>192) cg = 212;
                        if(cb>128) cb = 192;
                        blendpixel(vid, nx, ny, cr, cg, cb, 255);
                        blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
                        blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
                        blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
                        blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
                        blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
                        blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
                        blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
                        blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
                    }
                    // Older Code
                    /*if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr = parts[i].life / 4;
                        cg = parts[i].life / 16;
                        cb = parts[i].life / 32;
                        if(cr>255) cr = 255;
                        if(cg>192) cg = 212;
                        if(cb>128) cb = 192;
                        x = nx/CELL;
                        y = ny/CELL;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                    }
                    else
                    {
                        cr = parts[i].life * 8;
                        cg = parts[i].life * 2;
                        cb = parts[i].life;
                        if(cr>255) cr = 255;
                        if(cg>192) cg = 212;
                        if(cb>128) cb = 192;
                        blendpixel(vid, nx, ny, cr, cg, cb, 255);
                        blendpixel(vid, nx+1, ny, cr, cg, cb, 96);
                        blendpixel(vid, nx-1, ny, cr, cg, cb, 96);
                        blendpixel(vid, nx, ny+1, cr, cg, cb, 96);
                        blendpixel(vid, nx, ny-1, cr, cg, cb, 96);
                        blendpixel(vid, nx+1, ny-1, cr, cg, cb, 32);
                        blendpixel(vid, nx-1, ny+1, cr, cg, cb, 32);
                        blendpixel(vid, nx+1, ny+1, cr, cg, cb, 32);
                        blendpixel(vid, nx-1, ny-1, cr, cg, cb, 32);
                    }*/
                }
                else if(t==PT_LAVA && parts[i].life)
                {
                    cr = parts[i].life * 2 + 0xE0;
                    cg = parts[i].life * 1 + 0x50;
                    cb = parts[i].life/2 + 0x10;
                    if(cr>255) cr = 255;
                    if(cg>192) cg = 192;
                    if(cb>128) cb = 128;
                    blendpixel(vid, nx, ny, cr, cg, cb, 255);
                    blendpixel(vid, nx+1, ny, cr, cg, cb, 64);
                    blendpixel(vid, nx-1, ny, cr, cg, cb, 64);
                    blendpixel(vid, nx, ny+1, cr, cg, cb, 64);
                    blendpixel(vid, nx, ny-1, cr, cg, cb, 64);
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        cr /= 32;
                        cg /= 32;
                        cb /= 32;
                        x = nx/CELL;
                        y = ny/CELL;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                    }
                }
                else if(t==PT_LAVA || t==PT_SPRK)
                {
                    vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
                    if(cmode == 3 || cmode==4 || cmode==6)
                    {
                        if(t == PT_LAVA)
                        {
                            cr = 3;
                            cg = i%2;
                            cb = 0;
                        }
                        else
                        {
                            cr = 8;
                            cg = 12;
                            cb = 16;
                        }
                        x = nx/CELL;
                        y = ny/CELL;
                        cr += fire_r[y][x];
                        if(cr > 255) cr = 255;
                        fire_r[y][x] = cr;
                        cg += fire_g[y][x];
                        if(cg > 255) cg = 255;
                        fire_g[y][x] = cg;
                        cb += fire_b[y][x];
                        if(cb > 255) cb = 255;
                        fire_b[y][x] = cb;
                    }
                }
                else
                    vid[ny*(XRES+BARSIZE)+nx] = ptypes[t].pcolors;
            }
            else
            {
                float ttemp = parts[i].temp+(-MIN_TEMP);
                int caddress = restrict_flt((int)( restrict_flt(ttemp, 0.0f, MAX_TEMP+(-MIN_TEMP)) / ((MAX_TEMP+(-MIN_TEMP))/1024) ) *3, 0.0f, (1024.0f*3)-3);
                uint8 R = color_data[caddress];
                uint8 G = color_data[caddress+1];
                uint8 B = color_data[caddress+2];

                if(t==PT_STKM)  //Stick man should be visible in heat mode
                {
                    char buff[10];  //Buffer for HP

                    if(mousex>(nx-3) && mousex<(nx+3) && mousey<(ny+3) && mousey>(ny-3))  //If mous is in the head
                    {
                        sprintf(buff, "%3d", parts[i].life);  //Show HP
                        drawtext(vid, mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
                    }

                    for(r=-2; r<=1; r++)
                    {
                        s = XRES+BARSIZE;
                        vid[(ny-2)*s+nx+r] = PIXRGB (R, G, B);
                        vid[(ny+2)*s+nx+r+1] =  PIXRGB (R, G, B);
                        vid[(ny+r+1)*s+nx-2] =  PIXRGB (R, G, B);
                        vid[(ny+r)*s+nx+2] =  PIXRGB (R, G, B);
                    }
                    draw_line(vid , nx, ny+3, player[3], player[4], R, G, B, s);
                    draw_line(vid , player[3], player[4], player[7], player[8], R, G, B, s);
                    draw_line(vid , nx, ny+3, player[11], player[12], R, G, B, s);
                    draw_line(vid , player[11], player[12], player[15], player[16], R, G, B, s);

                    isplayer = 1;  //It's a secret. Tssss...
                }
                else
                {
                    vid[ny*(XRES+BARSIZE)+nx] = PIXRGB(R, G, B);
                    //blendpixel(vid, nx+1, ny, R, G, B, 255);
                }
            }
            if(cmode == 4&&t!=PT_FIRE&&t!=PT_PLSM&&t!=PT_HFLM&&t!=PT_NONE&&t!=PT_ACID&&t!=PT_LCRY&&t!=PT_GLOW&&t!=PT_SWCH&&t!=PT_SMKE&&t!=PT_WTRV&&!(t==PT_FIRW&&parts[i].tmp==3))
            {
                if(t==PT_PHOT) {
                    cg = 0;
                    cb = 0;
                    cr = 0;
                    for(x=0; x<12; x++) {
                        cr += (parts[i].ctype >> (x+18)) & 1;
                        cb += (parts[i].ctype >>  x)     & 1;
                    }
                    for(x=0; x<14; x++)
                        cg += (parts[i].ctype >> (x+9))  & 1;
                    x = 624/(cr+cg+cb+1);
                    cr *= x;
                    cg *= x;
                    cb *= x;
                    cr = cr>255?255:cr;
                    cg = cg>255?255:cg;
                    cb = cb>255?255:cb;
                } else {
                    cr = PIXR(ptypes[t].pcolors);
                    cg = PIXG(ptypes[t].pcolors);
                    cb = PIXB(ptypes[t].pcolors);
                }

                //if(vid[(ny-1)*YRES+(nx-1)]!=0){
                //	blendpixel(vid, nx, ny-1, R, G, B, 46);
                //}

                blendpixel(vid, nx+1, ny, cr, cg, cb, 223);
                blendpixel(vid, nx-1, ny, cr, cg, cb, 223);
                blendpixel(vid, nx, ny+1, cr, cg, cb, 223);
                blendpixel(vid, nx, ny-1, cr, cg, cb, 223);

                blendpixel(vid, nx+1, ny-1, cr, cg, cb, 112);
                blendpixel(vid, nx-1, ny-1, cr, cg, cb, 112);
                blendpixel(vid, nx+1, ny+1, cr, cg, cb, 112);
                blendpixel(vid, nx-1, ny+1, cr, cg, cb, 112);
            }
        }
#endif
    }
#ifdef OpenGL
    glFlush ();
#endif

}

void render_signs(pixel *vid_buf)
{
    int i, j, x, y, w, h, dx, dy;
    char buff[30];  //Buffer
    for(i=0; i<MAXSIGNS; i++)
        if(signs[i].text[0])
        {
            get_sign_pos(i, &x, &y, &w, &h);
            clearrect(vid_buf, x, y, w, h);
            drawrect(vid_buf, x, y, w, h, 192, 192, 192, 255);

            //Displaying special information
            if(strcmp(signs[i].text, "{p}")==0)
            {
                sprintf(buff, "Pressure: %3.2f", pv[signs[i].y/CELL][signs[i].x/CELL]);  //...pressure
                drawtext(vid_buf, x+3, y+3, buff, 255, 255, 255, 255);
            }

            if(strcmp(signs[i].text, "{t}")==0)
            {
                if((pmap[signs[i].y][signs[i].x]>>8)>0 && (pmap[signs[i].y][signs[i].x]>>8)<NPART)
                    sprintf(buff, "Temp: %4.2f", parts[pmap[signs[i].y][signs[i].x]>>8].temp-273.15);  //...tempirature
                else
                    sprintf(buff, "Temp: 0.00");  //...tempirature
                drawtext(vid_buf, x+3, y+3, buff, 255, 255, 255, 255);
            }

            //Usual text
            if(strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}"))
                drawtext(vid_buf, x+3, y+3, signs[i].text, 255, 255, 255, 255);
            x = signs[i].x;
            y = signs[i].y;
            dx = 1 - signs[i].ju;
            dy = (signs[i].y > 18) ? -1 : 1;
            for(j=0; j<4; j++)
            {
                drawpixel(vid_buf, x, y, 192, 192, 192, 255);
                x+=dx;
                y+=dy;
            }
        }
}

void render_fire(pixel *dst)
{
    int i,j,x,y,r,g,b;
    for(j=0; j<YRES/CELL; j++)
        for(i=0; i<XRES/CELL; i++)
        {
            r = fire_r[j][i];
            g = fire_g[j][i];
            b = fire_b[j][i];
            if(r || g || b)
                for(y=-CELL+1; y<2*CELL; y++)
                    for(x=-CELL+1; x<2*CELL; x++)
                        addpixel(dst, i*CELL+x, j*CELL+y, r, g, b, fire_alpha[y+CELL][x+CELL]);
            for(y=-1; y<2; y++)
                for(x=-1; x<2; x++)
                    if(i+x>=0 && j+y>=0 && i+x<XRES/CELL && j+y<YRES/CELL && (x || y))
                    {
                        r += fire_r[j+y][i+x] / 8;
                        g += fire_g[j+y][i+x] / 8;
                        b += fire_b[j+y][i+x] / 8;
                    }
            r /= 2;
            g /= 2;
            b /= 2;
            fire_r[j][i] = r>4 ? r-4 : 0;
            fire_g[j][i] = g>4 ? g-4 : 0;
            fire_b[j][i] = b>4 ? b-4 : 0;
        }
}

void prepare_alpha(void)
{
    int x,y,i,j;
    float temp[CELL*3][CELL*3];
    memset(temp, 0, sizeof(temp));
    for(x=0; x<CELL; x++)
        for(y=0; y<CELL; y++)
            for(i=-CELL; i<CELL; i++)
                for(j=-CELL; j<CELL; j++)
                    temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
    for(x=0; x<CELL*3; x++)
        for(y=0; y<CELL*3; y++)
            fire_alpha[y][x] = (int)(255.0f*temp[y][x]/(CELL*CELL));
}

pixel *render_packed_rgb(void *image, int width, int height, int cmp_size)
{
    unsigned char *tmp;
    pixel *res;
    int i;

    tmp = malloc(width*height*3);
    if(!tmp)
        return NULL;
    res = malloc(width*height*PIXELSIZE);
    if(!res)
    {
        free(tmp);
        return NULL;
    }

    i = width*height*3;
    if(BZ2_bzBuffToBuffDecompress((char *)tmp, (unsigned *)&i, (char *)image, cmp_size, 0, 0))
    {
        free(res);
        free(tmp);
        return NULL;
    }

    for(i=0; i<width*height; i++)
        res[i] = PIXRGB(tmp[3*i], tmp[3*i+1], tmp[3*i+2]);

    free(tmp);
    return res;
}

void draw_image(pixel *vid, pixel *img, int x, int y, int w, int h, int a)
{
    int i, j, r, g, b;
    for(j=0; j<h; j++)
        for(i=0; i<w; i++)
        {
            r = PIXR(*img);
            g = PIXG(*img);
            b = PIXB(*img);
            drawpixel(vid, x+i, y+j, r, g, b, a);
            img++;
        }
}

void dim_copy(pixel *dst, pixel *src)
{
    int i,r,g,b;
    for(i=0; i<XRES*YRES; i++)
    {
        r = PIXR(src[i]);
        g = PIXG(src[i]);
        b = PIXB(src[i]);
        if(r>0)
            r--;
        if(g>0)
            g--;
        if(b>0)
            b--;
        dst[i] = PIXRGB(r,g,b);
    }
}

void render_zoom(pixel *img)
{
    int x, y, i, j;
    pixel pix;
    drawrect(img, zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2, 192, 192, 192, 255);
    drawrect(img, zoom_wx-1, zoom_wy-1, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR, 0, 0, 0, 255);
    clearrect(img, zoom_wx, zoom_wy, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR);
    for(j=0; j<ZSIZE; j++)
        for(i=0; i<ZSIZE; i++)
        {
            pix = img[(j+zoom_y)*(XRES+BARSIZE)+(i+zoom_x)];
            for(y=0; y<ZFACTOR-1; y++)
                for(x=0; x<ZFACTOR-1; x++)
                    img[(j*ZFACTOR+y+zoom_wy)*(XRES+BARSIZE)+(i*ZFACTOR+x+zoom_wx)] = pix;
        }
    if(zoom_en)
    {
        for(j=-1; j<=ZSIZE; j++)
        {
            xor_pixel(zoom_x+j, zoom_y-1, img);
            xor_pixel(zoom_x+j, zoom_y+ZSIZE, img);
        }
        for(j=0; j<ZSIZE; j++)
        {
            xor_pixel(zoom_x-1, zoom_y+j, img);
            xor_pixel(zoom_x+ZSIZE, zoom_y+j, img);
        }
    }
}

pixel *prerender_save(void *save, int size, int *width, int *height)
{
    unsigned char *d,*c=save;
    int i,j,k,x,y,rx,ry,p=0;
    int bw,bh,w,h,new_format = 0;
    pixel *fb;

    if(size<16)
        return NULL;
    if(!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
        return NULL;
    if(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) {
        new_format = 1;
    }
    if(c[4]>SAVE_VERSION)
        return NULL;

    bw = c[6];
    bh = c[7];
    w = bw*CELL;
    h = bh*CELL;

    if(c[5]!=CELL)
        return NULL;

    i = (unsigned)c[8];
    i |= ((unsigned)c[9])<<8;
    i |= ((unsigned)c[10])<<16;
    i |= ((unsigned)c[11])<<24;
    d = malloc(i);
    if(!d)
        return NULL;
    fb = calloc(w*h, PIXELSIZE);
    if(!fb)
    {
        free(d);
        return NULL;
    }

    if(BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
        goto corrupt;
    size = i;

    if(size < bw*bh)
        goto corrupt;

    k = 0;
    for(y=0; y<bh; y++)
        for(x=0; x<bw; x++)
        {
            rx = x*CELL;
            ry = y*CELL;
            switch(d[p])
            {
            case 1:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
                break;
            case 2:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
                break;
            case 3:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(j%2) && !(i%2))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
                break;
            case 4:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0x8080FF);
                k++;
                break;
            case 6:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080);
                break;
            case 7:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(i&j&1))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
                break;
            case 8:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(j%2) && !(i%2))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xC0C0C0);
                        else
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0x808080);
                break;
            }
            p++;
        }
    p += 2*k;
    if(p>=size)
        goto corrupt;

    for(y=0; y<h; y++)
        for(x=0; x<w; x++)
        {
            if(p >= size)
                goto corrupt;
            j=d[p++];
            if(j<PT_NUM && j>0)
            {
                if(j==PT_STKM)  //Stickman should be drawed another way
                {
                    //Stickman drawing
                    for(k=-2; k<=1; k++)
                    {
                        fb[(y-2)*w+x+k] = PIXRGB(255, 224, 178);
                        fb[(y+2)*w+x+k+1] = PIXRGB(255, 224, 178);
                        fb[(y+k+1)*w+x-2] = PIXRGB(255, 224, 178);
                        fb[(y+k)*w+x+2] = PIXRGB(255, 224, 178);
                    }
                    draw_line(fb , x, y+3, x-1, y+6, 255, 255, 255, w);
                    draw_line(fb , x-1, y+6, x-3, y+12, 255, 255, 255, w);
                    draw_line(fb , x, y+3, x+1, y+6, 255, 255, 255, w);
                    draw_line(fb , x+1, y+6, x+3, y+12, 255, 255, 255, w);
                }
                else
                    fb[y*w+x] = ptypes[j].pcolors;
            }
        }

    free(d);
    *width = w;
    *height = h;
    return fb;

corrupt:
    free(d);
    free(fb);
    return NULL;
}

int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl)
{
    unsigned char *d,*c=thumb;
    int i,j,x,y,a,t,r,g,b,sx,sy;

    if(bzip2)
    {
        if(size<16)
            return 1;
        if(c[3]!=0x74 || c[2]!=0x49 || c[1]!=0x68 || c[0]!=0x53)
            return 1;
        if(c[4]>PT_NUM)
            return 2;
        if(c[5]!=CELL || c[6]!=XRES/CELL || c[7]!=YRES/CELL)
            return 3;
        i = XRES*YRES;
        d = malloc(i);
        if(!d)
            return 1;

        if(BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+8), size-8, 0, 0))
            return 1;
        size = i;
    }
    else
        d = c;

    if(size < XRES*YRES)
    {
        if(bzip2)
            free(d);
        return 1;
    }

    sy = 0;
    for(y=0; y+scl<=YRES; y+=scl)
    {
        sx = 0;
        for(x=0; x+scl<=XRES; x+=scl)
        {
            a = 0;
            r = g = b = 0;
            for(j=0; j<scl; j++)
                for(i=0; i<scl; i++)
                {
                    t = d[(y+j)*XRES+(x+i)];
                    if(t==0xFF)
                    {
                        r += 256;
                        g += 256;
                        b += 256;
                        a += 2;
                    }
                    else if(t)
                    {
                        if(t>=PT_NUM)
                            goto corrupt;
                        r += PIXR(ptypes[t].pcolors);
                        g += PIXG(ptypes[t].pcolors);
                        b += PIXB(ptypes[t].pcolors);
                        a ++;
                    }
                }
            if(a)
            {
                a = 256/a;
                r = (r*a)>>8;
                g = (g*a)>>8;
                b = (b*a)>>8;
            }

            drawpixel(vid_buf, px+sx, py+sy, r, g, b, 255);
            sx++;
        }
        sy++;
    }

    if(bzip2)
        free(d);
    return 0;

corrupt:
    if(bzip2)
        free(d);
    return 1;
}

void render_cursor(pixel *vid, int x, int y, int t, int r)
{
    int i,j,c;
    if(t<PT_NUM||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM)
    {
        if(r<=0)
            xor_pixel(x, y, vid);
        else
            for(j=0; j<=r; j++)
                for(i=0; i<=r; i++)
                    if(i*i+j*j<=r*r && ((i+1)*(i+1)+j*j>r*r || i*i+(j+1)*(j+1)>r*r))
                    {
                        xor_pixel(x+i, y+j, vid);
                        if(j) xor_pixel(x+i, y-j, vid);
                        if(i) xor_pixel(x-i, y+j, vid);
                        if(i&&j) xor_pixel(x-i, y-j, vid);
                    }
    }
    else
    {
        int tc;
        c = (r/CELL) * CELL;
        x = (x/CELL) * CELL;
        y = (y/CELL) * CELL;

        tc = !((c%(CELL*2))==0);

        x -= c/2;
        y -= c/2;

        x += tc*(CELL/2);
        y += tc*(CELL/2);

        for(i=0; i<CELL+c; i++)
        {
            xor_pixel(x+i, y, vid);
            xor_pixel(x+i, y+CELL+c-1, vid);
        }
        for(i=1; i<CELL+c-1; i++)
        {
            xor_pixel(x, y+i, vid);
            xor_pixel(x+CELL+c-1, y+i, vid);
        }
    }
}

void sdl_open(void)
{
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
#ifdef OpenGL
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_OPENGL);
    SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
    Enable2D ();
    //   glBegin(GL_TRIANGLES);
    //   glColor3ub (255, 0, 0);
    //   glVertex2d (0, 0);
    //   glColor3ub (0, 255, 0);
    //   glVertex2d (640,0);
    //    glColor3ub (0, 0, 255);
    //    glVertex2d (50, 50);
    //glEnd ();
    //glFlush ();
#else
#ifdef PIX16
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,16,SDL_SWSURFACE);
#else
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_SWSURFACE);
#endif
#endif
    if(!sdl_scrn)
    {
        fprintf(stderr, "Creating window: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
    sdl_seticon();
    SDL_EnableUNICODE(1);
    //SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

#ifdef OpenGL
void Enable2D ()
{
    int ViewPort[4]; //Holds screen info

    glGetIntegerv (GL_VIEWPORT, ViewPort);

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();

    glOrtho (0, ViewPort[2], ViewPort[3], 0, -1, 1);
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint( GL_POINT_SMOOTH_HINT, GL_NICEST);
}
void RenderScene ()
{
    SDL_GL_SwapBuffers ();
}
void ClearScreen()
{
    glClear(GL_COLOR_BUFFER_BIT);
}
#endif
