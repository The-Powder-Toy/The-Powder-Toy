#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>

#include <defines.h>
#include <air.h>
#include <powder.h>
#include <graphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>

//Prototypes for intern use only
void Renderer_Intern_BlitOne();
void Renderer_Intern_BlitTwo();
void Renderer_Intern_DimmedCopy(pixel *dst, pixel *src);
void Renderer_Intern_AddPixel(int x, int y, int r, int g, int b, int a);

unsigned int fire_alpha[CELL*3][CELL*3];
unsigned char PersistentTick=0;
SDL_Surface *sdl_scrn;
int sdl_scale = 1;
pixel *PrimaryBuffer;
pixel *SecondaryBuffer;
pixel *StateMemory;

//Intern functions
void Renderer_Intern_BlitOne()
{
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    if(SDL_MUSTLOCK(sdl_scrn))
        SDL_UnlockSurface(sdl_scrn);
    SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void Renderer_Intern_BlitTwo()
{
    pixel *dst;
    int j;
    int i,k;
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    dst=(pixel *)sdl_scrn->pixels;
    for(j=0; j<(YRES+MENUSIZE); j++)
    {
        for(k=0; k<sdl_scale; k++)
        {
            for(i=0; i<(XRES+BARSIZE); i++)
            {
                dst[i*2]=PrimaryBuffer[i];
                dst[i*2+1]=PrimaryBuffer[i];
            }
            dst+=sdl_scrn->pitch/PIXELSIZE;
        }
        PrimaryBuffer+=(XRES+BARSIZE);
    }
    PrimaryBuffer-=(XRES+BARSIZE)*(YRES+MENUSIZE);
    if(SDL_MUSTLOCK(sdl_scrn))
        SDL_UnlockSurface(sdl_scrn);
    SDL_UpdateRect(sdl_scrn,0,0,0,0);
}

void Renderer_Intern_DimmedCopy(pixel *dst, pixel *src)
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

void Renderer_Intern_AddPixel(int x, int y, int r, int g, int b, int a)
{
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    pixel t = PrimaryBuffer[y*(XRES+BARSIZE)+x];
    r = (a*r + 255*PIXR(t)) >> 8;
    g = (a*g + 255*PIXG(t)) >> 8;
    b = (a*b + 255*PIXB(t)) >> 8;
    if(r>255)
        r = 255;
    if(g>255)
        g = 255;
    if(b>255)
        b = 255;
    PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}


void Renderer_Init()
{
    SecondaryBuffer = (pixel*) calloc(XRES*YRES, PIXELSIZE);
    StateMemory = (pixel*) calloc((XRES+BARSIZE)*(YRES+MENUSIZE)*4, PIXELSIZE);
    PrimaryBuffer = (pixel*) calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
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

    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_SWSURFACE);
    if(!sdl_scrn)
    {
        fprintf(stderr, "Creating window: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
    #ifdef WIN32
        //SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon_w32, 32, 32, 32, 128, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        //SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
    #else
        #ifdef MACOSX
            //SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon_w32, 32, 32, 32, 128, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            //SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
        #else
            SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 128, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            SDL_WM_SetIcon(icon, NULL/*app_icon_mask*/);
        #endif
    #endif
    SDL_EnableUNICODE(1);
}

void Renderer_PrepareScreen()
{
    if(cmode==2)
    {
        memcpy(PrimaryBuffer, SecondaryBuffer, XRES*YRES*PIXELSIZE);
        memset(PrimaryBuffer+(XRES*YRES), 0, ((XRES+BARSIZE)*YRES*PIXELSIZE)-(XRES*YRES*PIXELSIZE));
    }
    else
    {
        memset(PrimaryBuffer, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
    }
}

void Renderer_ClearSecondaryBuffer()
{
    memset(SecondaryBuffer, 0, XRES*YRES*PIXELSIZE);
}

void Renderer_Display()
{
    if(sdl_scale == 2)
        Renderer_Intern_BlitTwo();
    else
        Renderer_Intern_BlitOne();
}

void Renderer_SaveState(unsigned char slot)
{
    memcpy(StateMemory+(slot*(XRES+BARSIZE)*(YRES+MENUSIZE)), PrimaryBuffer, (XRES+BARSIZE)*(YRES+MENUSIZE)*PIXELSIZE);
}

void Renderer_RecallState(unsigned char slot)
{
    memcpy(PrimaryBuffer, StateMemory+(slot*(XRES+BARSIZE)*(YRES+MENUSIZE)), ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
}

void Renderer_SaveScreenshot(int w, int h, int pitch)
{
    char frame_name[32];
    int j,i;
    unsigned char c[3];
    FILE *f;
    sprintf(frame_name,"frame%04d.ppm",frame_idx);
    f=fopen(frame_name,"wb");
    fprintf(f,"P6\n%d %d\n255\n",w,h);
    for(j=0; j<h; j++)
    {
        for(i=0; i<w; i++)
        {
            c[0] = PIXR(PrimaryBuffer[i]);
            c[1] = PIXG(PrimaryBuffer[i]);
            c[2] = PIXB(PrimaryBuffer[i]);
            fwrite(c,3,1,f);
        }
        PrimaryBuffer+=pitch;
    }
    PrimaryBuffer-=pitch*h;
    fclose(f);
    frame_idx++;
}

void Renderer_DrawPixel(int x, int y, pixel color)
{
    PrimaryBuffer[(XRES+BARSIZE)*y+x] = color;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void Renderer_BlendPixel(int x, int y, int r, int g, int b, int a)
#else
inline void Renderer_BlendPixel(int x, int y, int r, int g, int b, int a)
#endif
{
    if(x<0 || y<0 || x>=XRES+BARSIZE || y>=YRES+MENUSIZE)
        return;
    if(a<255)
    {
        pixel t = PrimaryBuffer[y*(XRES+BARSIZE)+x];
        r = (a*r + (255-a)*PIXR(t)) >> 8;
        g = (a*g + (255-a)*PIXG(t)) >> 8;
        b = (a*b + (255-a)*PIXB(t)) >> 8;
    }
    PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
}

void Renderer_DrawBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
    Renderer_BlendPixel(x, y, cr, cg, cb, 255);
    Renderer_BlendPixel(x+1, y, cr, cg, cb, 112);
    Renderer_BlendPixel(x-1, y, cr, cg, cb, 112);
    Renderer_BlendPixel(x, y+1, cr, cg, cb, 112);
    Renderer_BlendPixel(x, y-1, cr, cg, cb, 112);
    Renderer_BlendPixel(x+1, y-1, cr, cg, cb, 64);
    Renderer_BlendPixel(x-1, y-1, cr, cg, cb, 64);
    Renderer_BlendPixel(x+1, y+1, cr, cg, cb, 64);
    Renderer_BlendPixel(x-1, y+1, cr, cg, cb, 64);
}

void Renderer_DrawDots(int x, int y, int h, int r, int g, int b, int a)
{
    int i;
    for(i=0; i<=h; i+=2)
        Renderer_BlendPixel(x, y+i, r, g, b, a);
}

void Renderer_DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
    int dx, dy, i, sx, sy, check, e, x, y, nr, ng, nb ;
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
        if(a<255)
        {
            pixel t = PrimaryBuffer[y*(XRES+BARSIZE)+x];
            nr = ((a*r + (255-a)*PIXR(t)) / 255);
            ng = ((a*g + (255-a)*PIXG(t)) / 255);
            nb = ((a*b + (255-a)*PIXB(t)) / 255);
            PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXRGB(nr,ng,nb);
        }
        else
            PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXRGB(r,g,b);
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

void Renderer_DrawRectangle(int x, int y, int w, int h, int r, int g, int b, int a)
{
    int i;
    for(i=0; i<=w; i++)
    {
        Renderer_BlendPixel(x+i, y, r, g, b, a);
        Renderer_BlendPixel(x+i, y+h, r, g, b, a);
    }
    for(i=1; i<h; i++)
    {
        Renderer_BlendPixel(x, y+i, r, g, b, a);
        Renderer_BlendPixel(x+w, y+i, r, g, b, a);
    }
}

void Renderer_FillRectangle(int x, int y, int w, int h, int r, int g, int b, int a)
{
    int i,j;
    for(j=1; j<h; j++)
        for(i=1; i<w; i++)
            Renderer_BlendPixel(x+i, y+j, r, g, b, a);
}

void Renderer_ClearRectangle(int x, int y, int w, int h)
{
    int i;
    for(i=1; i<h; i++)
        memset(PrimaryBuffer+(x+1+(XRES+BARSIZE)*(y+i)), 0, PIXELSIZE*(w-1));
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int Renderer_DrawChar(int x, int y, int c, int r, int g, int b, int a)
#else
inline int Renderer_DrawChar(int x, int y, int c, int r, int g, int b, int a)
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
            Renderer_BlendPixel(x+i, y+j, r, g, b, ((ba&3)*a)/3);
            ba >>= 2;
            bn -= 2;
        }
    return x + w;
}

void Renderer_XORPixel(int x, int y)
{
    int c;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    c = PrimaryBuffer[y*(XRES+BARSIZE)+x];
    c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
    if(c<512)
        PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXPACK(0xC0C0C0);
    else
        PrimaryBuffer[y*(XRES+BARSIZE)+x] = PIXPACK(0x404040);
}

void Renderer_XORLine(int x1, int y1, int x2, int y2)
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
            Renderer_XORPixel(y, x);
        else
            Renderer_XORPixel(x, y);
        e += de;
        if(e >= 0.5f)
        {
            y += sy;
            e -= 1.0f;
        }
    }
}

void Renderer_XORRectangle(int x, int y, int w, int h)
{
    int i;
    for(i=0; i<w; i+=2)
    {
        Renderer_XORPixel(x+i, y);
        Renderer_XORPixel(x+i, y+h-1);
    }
    for(i=2; i<h; i+=2)
    {
        Renderer_XORPixel(x, y+i);
        Renderer_XORPixel(x+w-1, y+i);
    }
}

void Renderer_DrawZoom()
{
    int x, y, i, j;
    pixel pix;
    Renderer_DrawRectangle(zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2, 192, 192, 192, 255);
    Renderer_DrawRectangle(zoom_wx-1, zoom_wy-1, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR, 0, 0, 0, 255);
    Renderer_ClearRectangle(zoom_wx, zoom_wy, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR);
    for(j=0; j<ZSIZE; j++)
        for(i=0; i<ZSIZE; i++)
        {
            pix = PrimaryBuffer[(j+zoom_y)*(XRES+BARSIZE)+(i+zoom_x)];
            for(y=0; y<ZFACTOR-1; y++)
                for(x=0; x<ZFACTOR-1; x++)
                    PrimaryBuffer[(j*ZFACTOR+y+zoom_wy)*(XRES+BARSIZE)+(i*ZFACTOR+x+zoom_wx)] = pix;
        }
    if(zoom_en)
    {
        for(j=-1; j<=ZSIZE; j++)
        {
            Renderer_XORPixel(zoom_x+j, zoom_y-1);
            Renderer_XORPixel(zoom_x+j, zoom_y+ZSIZE);
        }
        for(j=0; j<ZSIZE; j++)
        {
            Renderer_XORPixel(zoom_x-1, zoom_y+j);
            Renderer_XORPixel(zoom_x+ZSIZE, zoom_y+j);
        }
    }
}

void Renderer_ClearMenu()
{
    memset(PrimaryBuffer+((XRES+BARSIZE)*YRES), 0, (PIXELSIZE*(XRES+BARSIZE))*MENUSIZE);
}

void Renderer_GrabPersistent()
{
    if(!PersistentTick)
    {
        Renderer_Intern_DimmedCopy(SecondaryBuffer, PrimaryBuffer);
    }
    else
    {
        memcpy(SecondaryBuffer, PrimaryBuffer, XRES*YRES*PIXELSIZE);
    }
    PersistentTick = (PersistentTick+1) % 3;
}

void Renderer_DrawImage(pixel *img, int x, int y, int w, int h, int a)
{
    int i, j, r, g, b;
    for(j=0; j<h; j++)
        for(i=0; i<w; i++)
        {
            r = PIXR(*img);
            g = PIXG(*img);
            b = PIXB(*img);
            Renderer_BlendPixel(x+i, y+j, r, g, b, a);
            img++;
        }
}

void Renderer_DrawAir()
{
    int x, y, i, j;
    pixel c;
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
                    Renderer_DrawPixel(x*CELL+i, y*CELL+j, c);
        }
}

void Renderer_DrawFire()
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
                        Renderer_Intern_AddPixel(i*CELL+x, j*CELL+y, r, g, b, fire_alpha[y+CELL][x+CELL]);
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
            if(r>4)
                fire_r[j][i] = r-4;
            else
                fire_r[j][i] = 0;
            if(g>4)
                fire_g[j][i] = g-4;
            else
                fire_g[j][i] = 0;
            if(b>4)
                fire_b[j][i] = b-4;
            else
                fire_b[j][i] = 0;
            //fire_r[j][i] = r>4 ? r-4 : 0;
            //fire_g[j][i] = g>4 ? g-4 : 0;
            //fire_b[j][i] = b>4 ? b-4 : 0;
        }
}