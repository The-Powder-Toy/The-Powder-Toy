#include <math.h>
#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <defines.h>
#include <air.h>
#include <powder.h>
#include <graphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>

//Prototypes for intern use only
pixel Renderer_Intern_GetPixel(int x, int y);

#define GLOWTEXSIZE 16
#define BLOBTEXSIZE 4

typedef void (APIENTRY * GL_BlendEquation)(GLenum);
GL_BlendEquation glBlendEquation = 0;

SDL_Surface *sdl_scrn;
GLuint GlowTexture = 0;
GLuint BlobTexture = 0;
int sdl_scale = 1;
unsigned char PersistentTick=0;
unsigned int *StateMemory;
unsigned int *SecondaryBuffer;

//Intern functions
pixel Renderer_Intern_GetPixel(int x, int y)
{
    pixel result;
    glReadPixels(x, (YRES+MENUSIZE)-y-1, 1, 1, GL_BGR, GL_UNSIGNED_BYTE, &result);
    return result;
}


void Renderer_Init()
{
    StateMemory = (unsigned int)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), 3);
    SecondaryBuffer = (unsigned int)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), 3);
    int x,y,i,j;
    float temp[CELL*3][CELL*3];
    memset(temp, 0, sizeof(temp));
    for(x=0; x<CELL; x++)
        for(y=0; y<CELL; y++)
            for(i=-CELL; i<CELL; i++)
                for(j=-CELL; j<CELL; j++)
                    temp[y+CELL+j][x+CELL+i] += expf(-0.1f*(i*i+j*j));
    unsigned int fire_alpha[CELL*3][CELL*3];
    for(x=0; x<CELL*3; x++)
        for(y=0; y<CELL*3; y++)
            fire_alpha[y][x] = (int)(255.0f*temp[y][x]/(CELL*CELL));
    i = (GLOWTEXSIZE*GLOWTEXSIZE)-1;
    unsigned char GlowAlphaTmp[GLOWTEXSIZE*GLOWTEXSIZE];
    for(y=GLOWTEXSIZE; y>0; y--)
        for(x=1; x<(GLOWTEXSIZE+1); x++)
        {
            GlowAlphaTmp[i] = (y<(3*CELL-1) && x<(3*CELL+1)) ? fire_alpha[y][x] : 0;
            i--;
        }
    unsigned char BlobAlphaTmp[] = {
     64, 112,  64,   0,
    112, 255, 112,   0,
     64, 112,  64,   0,
      0,   0,   0,   0
    };
    if(SDL_Init(SDL_INIT_VIDEO)<0)
    {
        fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale + BARSIZE*sdl_scale,YRES*sdl_scale + MENUSIZE*sdl_scale,32,SDL_OPENGL);
    if(!sdl_scrn)
    {
        fprintf(stderr, "Creating window: %s\n", SDL_GetError());
        exit(1);
    }
    glBlendEquation=(GL_BlendEquation) SDL_GL_GetProcAddress("glBlendEquation");
    if(!glBlendEquation){
        fprintf(stderr, "Blend Equation Extensions not present.\n");
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
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, (XRES+BARSIZE)*sdl_scale,(YRES+MENUSIZE)*sdl_scale-1, -1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glReadBuffer(GL_BACK);
    glDrawBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT,2);
    glPixelStorei(GL_UNPACK_ALIGNMENT,2);
    glRasterPos2i(0,YRES-2);
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    glGenTextures(1, &BlobTexture);
    glBindTexture(GL_TEXTURE_2D, BlobTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, BLOBTEXSIZE, BLOBTEXSIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &BlobAlphaTmp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    
    glGenTextures(1, &GlowTexture);
    glBindTexture(GL_TEXTURE_2D, GlowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, GLOWTEXSIZE, GLOWTEXSIZE, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &GlowAlphaTmp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void Renderer_PrepareScreen()
{
    if(cmode==2)
    {
        glRasterPos2i(0,YRES-1);
        if(!PersistentTick)
        {
            glDrawPixels(XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, SecondaryBuffer);
            glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
            Renderer_FillRectangle(-1,-1,XRES+2,YRES+2,255,255,255,1);
            glBlendEquation(GL_FUNC_ADD);
        }
        else
        {
            glDrawPixels(XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, SecondaryBuffer);
        }
        PersistentTick = (PersistentTick+1) % 3;
        Renderer_ClearRectangle(-1,YRES-11,XRES+2,12);
    }
    else
    {
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void Renderer_ClearSecondaryBuffer()
{
    memset(SecondaryBuffer, 0, (XRES+BARSIZE)*(YRES+MENUSIZE)*3);
}

void Renderer_Display(int x, int y, int w, int h, int pitch)
{
    SDL_GL_SwapBuffers();
}

void Renderer_SaveState()
{
    glReadPixels(0, 0, XRES+BARSIZE, YRES+MENUSIZE, GL_RGB, GL_UNSIGNED_BYTE, StateMemory);
}

void Renderer_RecallState()
{
    glRasterPos2i(0,YRES+MENUSIZE-1);
    glDrawPixels(XRES+BARSIZE, YRES+MENUSIZE, GL_RGB, GL_UNSIGNED_BYTE, StateMemory);
}

void Renderer_SaveScreenshot(int w, int h, int pitch)
{
    unsigned int *dump;
    dump = (unsigned int)calloc(w*h, 3);
    glReadPixels(0, MENUSIZE, w, h, GL_RGB, GL_UNSIGNED_BYTE, dump);
    char frame_name[32];
    FILE *f;
    sprintf(frame_name,"frame%04d.ppm",frame_idx);
    f=fopen(frame_name,"wb");
    fprintf(f,"P6\n%d %d\n255\n",w,h);
    fwrite (dump, 3, w*h, f);
    fclose(f);
    frame_idx++;
    free(dump);
}

void Renderer_DrawPixel(int x, int y, pixel color)
{
    glBegin(GL_POINTS);
    glColor3ub(PIXR(color),PIXG(color),PIXB(color));
    glVertex2i(x, y);
    glEnd();
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void Renderer_BlendPixel(int x, int y, int r, int g, int b, int a)
#else
extern inline void Renderer_BlendPixel(int x, int y, int r, int g, int b, int a)
#endif
{
    glBegin(GL_POINTS);
    glColor4ub(r,g,b,a);
    glVertex2i(x, y);
    glEnd();
}

void Renderer_DrawBlob(int x, int y, unsigned char cr, unsigned char cg, unsigned char cb)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, BlobTexture);
    glBegin(GL_QUADS);
    glColor3ub(cr, cg, cb);
    glTexCoord2i(1,0);
    glVertex2i(x-2, y-2);
    glTexCoord2i(0,0);
    glVertex2i(x+BLOBTEXSIZE-2, y-2);
    glTexCoord2i(0,1);
    glVertex2i(x+BLOBTEXSIZE-2, y+BLOBTEXSIZE-2);
    glTexCoord2i(1,1);
    glVertex2i(x-2, y+BLOBTEXSIZE-2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void Renderer_DrawDots(int x, int y, int h, int r, int g, int b, int a)
{
    int i;
    glBegin(GL_POINTS);
    glColor4ub(r, g, b, a);
    for(i = 0; i <= h; i +=2)
        glVertex2i(x, y+i);
    glEnd();
}

void Renderer_DrawLine(int x1, int y1, int x2, int y2, int r, int g, int b, int a)
{
    glBegin(GL_LINES);
    glColor4ub(r,g,b,a);
    glVertex2i(x1, y1-1);
    glVertex2i(x2+1, y2);
    glEnd();
}

void Renderer_DrawRectangle(int x, int y, int w, int h, int r, int g, int b, int a)
{
    glBegin(GL_LINE_STRIP );
    glColor4ub(r, g, b, a);
    glVertex2i(x+1, y+h-1);
    glVertex2i(x+w+1, y+h-1);
    glVertex2i(x+w+1, y-1);
    glVertex2i(x+1, y-1);
    glVertex2i(x+1, y+h);
    glEnd();
}

void Renderer_FillRectangle(int x, int y, int w, int h, int r, int g, int b, int a)
{
    glBegin(GL_QUADS);
    glColor4ub(r, g, b, a);
    glVertex2i(x+1, y);
    glVertex2i(x+w, y);
    glVertex2i(x+w, y+h-1);
    glVertex2i(x+1, y+h-1);
    glEnd();
}

void Renderer_ClearRectangle(int x, int y, int w, int h)
{
    glBegin(GL_QUADS);
    glColor3ub(0,0,0);
    glVertex2i(x+1, y);
    glVertex2i(x+w, y);
    glVertex2i(x+w, y+h-1);
    glVertex2i(x+1, y+h-1);
    glEnd();
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int Renderer_DrawChar(int x, int y, int c, int r, int g, int b, int a)
#else
extern inline int Renderer_DrawChar(int x, int y, int c, int r, int g, int b, int a)
#endif
{
    int i, j, w, bn = 0, ba = 0;
    char *rp = font_data + font_ptrs[c];
    w = *(rp++);
    glBegin(GL_POINTS);
    for(j=0; j<FONT_H; j++)
        for(i=0; i<w; i++)
        {
            if(!bn)
            {
                ba = *(rp++);
                bn = 8;
            }
            glColor4ub(r, g, b, ((ba&3)*a)/3);
            glVertex2i(x+i, y+j);
            ba >>= 2;
            bn -= 2;
        }
    glEnd();
    return x + w;
}

void Renderer_XORPixel(int x, int y)
{
    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    int c = Renderer_Intern_GetPixel(x,y);
    c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
    glBegin(GL_POINTS);
    if(c<512)
        glColor3ub(0xC0, 0xC0, 0xC0);
    else
        glColor3ub(0x40, 0x40, 0x40);
    glVertex2i(x, y);
    glEnd();
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
    Renderer_DrawRectangle(zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2, 192, 192, 192, 255);
    Renderer_DrawRectangle(zoom_wx-1, zoom_wy-1, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR, 0, 0, 0, 255);
    Renderer_ClearRectangle(zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2);
    /*
    glRasterPos2i(zoom_wx,YRES+MENUSIZE-(ZSIZE*ZFACTOR)+85);
    glPixelZoom(ZFACTOR,ZFACTOR);
    glCopyPixels(zoom_x,YRES+MENUSIZE-ZSIZE-zoom_y,ZSIZE,ZSIZE,GL_COLOR);
    glPixelZoom(1,1);
    */
    int i, j=0, x, y;
    unsigned char Pixels[(ZSIZE)*(ZSIZE)*4];
    glReadPixels(zoom_x, (YRES+MENUSIZE)-ZSIZE-zoom_y, ZSIZE, ZSIZE, GL_RGBA, GL_UNSIGNED_BYTE, &Pixels);
    glBegin(GL_QUADS);
    for(i=0; i<(ZSIZE*ZSIZE*4); i+=4)
    {
        x=(j%ZSIZE)*ZFACTOR+zoom_wx;
        y=(ZSIZE-1-(j/ZSIZE))*ZFACTOR+zoom_wy;
        glColor3ub(Pixels[i], Pixels[i+1], Pixels[i+2]);
        glVertex2i(x, y-1);
        glVertex2i(x+ZFACTOR-1, y-1);
        glVertex2i(x+ZFACTOR-1, y+ZFACTOR-2);
        glVertex2i(x, y+ZFACTOR-2);
        j++;
    }
    glEnd();
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

void Renderer_GrabPersistent()
{
    glReadPixels(0, MENUSIZE, XRES, YRES, GL_RGB, GL_UNSIGNED_BYTE, SecondaryBuffer);
}

void Renderer_ClearMenu()
{
    Renderer_ClearRectangle(-1, YRES-2, XRES+BARSIZE+2, MENUSIZE+2);
}

void Renderer_DrawImage(pixel *img, int x, int y, int w, int h, int a)
{
    glRasterPos2i(x,y-1);
    glPixelZoom(1.0f,-1.0f);
    glDrawPixels(w, h, GL_BGRA, GL_UNSIGNED_BYTE, img);
    glPixelZoom(1.0f,1.0f);
}

void Renderer_DrawAir()
{
    int x, y, i, j;
    pixel c;
    glBegin(GL_QUADS);
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
                
            glColor4ub(PIXR(c),PIXG(c),PIXB(c), 255);
            glVertex2i(x*CELL, y*CELL-1);
            glVertex2i(x*CELL+CELL, y*CELL-1);
            glVertex2i(x*CELL+CELL, y*CELL+CELL-1);
            glVertex2i(x*CELL, y*CELL+CELL-1);
        }
    glEnd();
}

void Renderer_DrawFire()
{
    int i,j,x,y,r,g,b;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GlowTexture);
    glBegin(GL_QUADS);
    for(j=0; j<YRES/CELL; j++)
        for(i=0; i<XRES/CELL; i++)
        {
            r = fire_r[j][i];
            g = fire_g[j][i];
            b = fire_b[j][i];
            if(r || g || b)
            {
                glColor3ub(r, g, b);
                glTexCoord2i(1,0);
                glVertex2i(i*CELL-3, j*CELL-4);
                glTexCoord2i(0,0);
                glVertex2i(i*CELL+GLOWTEXSIZE-3, j*CELL-4);
                glTexCoord2i(0,1);
                glVertex2i(i*CELL+GLOWTEXSIZE-3, j*CELL+GLOWTEXSIZE-4);
                glTexCoord2i(1,1);
                glVertex2i(i*CELL-3, j*CELL+GLOWTEXSIZE-4);
            }
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
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}