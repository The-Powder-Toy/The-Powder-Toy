#include <math.h>

#include <renderer.h>
#include <defines.h>
#include <air.h>
#include <powder.h>
#include <graphics.h>
#define INCLUDE_FONTDATA
#include <font.h>
#include <misc.h>

unsigned char fire_r[YRES/CELL][XRES/CELL];
unsigned char fire_g[YRES/CELL][XRES/CELL];
unsigned char fire_b[YRES/CELL][XRES/CELL];

unsigned cmode = 3;
unsigned int frame_idx=0;

void Graphics_RenderGrid()
{
    int i;
    for(i=4;i<=XRES; i+=4)
        Renderer_DrawLine(i, 0, i, YRES, 128, 128, 128, 64);
    for(i=4;i<=YRES; i+=4)
        Renderer_DrawLine(0, i, XRES, i, 128, 128, 128, 64);
}

void Graphics_RenderWalls()
{
    int i, j, nx, ny, x, y, cr, cg, cb;
    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
        {
            if(bmap[y][x]==1)
                Renderer_FillRectangle(x*CELL-1, y*CELL-1, CELL+1, CELL+1, 0x80, 0x80, 0x80, 0xFF);
            if(bmap[y][x]==2)
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x808080));
            if(bmap[y][x]==3)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0xC0C0C0));
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==4)
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x8080FF));
            if(bmap[y][x]==6)
            {
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0xFF8080));
                if(emap[y][x])
                {
                    cr = 255;
                    cg = 32;
                    cb = 8;
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
            if(bmap[y][x]==7)
            {
                if(emap[y][x])
                {
                    cr = cg = cb = 128;
                    cr += fire_r[y][x];
                    if(cr > 255) cr = 255;
                    fire_r[y][x] = cr;
                    cg += fire_g[y][x];
                    if(cg > 255) cg = 255;
                    fire_g[y][x] = cg;
                    cb += fire_b[y][x];
                    if(cb > 255) cb = 255;
                    fire_b[y][x] = cb;
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(i&j&1)
                                Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x808080));
                }
                else
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(!(i&j&1))
                                Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x808080));
                }
            }
            if(bmap[y][x]==8)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                    {
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0xC0C0C0));
                        else
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x808080));
                    }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==11)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0xFFFF22));
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==9)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x3C3C3C));
                    }
                }
            }
            if(bmap[y][x]==13)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x579777));
                    }
                }
            }
            if(bmap[y][x]==10)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x575757));
                    }
                }
            }
            if(bmap[y][x]==12)
            {
                if(emap[y][x])
                {
                    for(j=0; j<CELL; j++)
                    {
                        for(i=(j)&1; i<CELL; i++)
                        {
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x242424));
                        }
                    }
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j)&1; i<CELL; i+=2)
                        {
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x000000));
                        }
                    }
                }
                else
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j)&1; i<CELL; i+=2)
                        {
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x242424));
                        }
                    }
                }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
        }
    
    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
            if(bmap[y][x]==5)
            {
                float lx,ly;
                lx = x*CELL + CELL*0.5f;
                ly = y*CELL + CELL*0.5f;
                for(int t=0; t<1024; t++)
                {
                    nx = (int)(lx+0.5f);
                    ny = (int)(ly+0.5f);
                    if(nx<0 || nx>=XRES || ny<0 || ny>=YRES)
                        break;
                    Renderer_BlendPixel(nx, ny, 255, 255, 255, 64);
                    i = nx/CELL;
                    j = ny/CELL;
                    lx += vx[j][i]*0.125f;
                    ly += vy[j][i]*0.125f;
                    if(bmap[j][i]==5 && i!=x && j!=y)
                        break;
                }
                Graphics_RenderText(x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
            }
}

void Graphics_RenderWallsBlob()
{
    int i, j, nx, ny, x, y, cr, cg, cb;
    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
        {
            if(bmap[y][x]==1)
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                    }
            if(bmap[y][x]==2)
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                    }
            if(bmap[y][x]==3)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0xC0, 0xC0, 0xC0);
                        }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==4)
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0xFF);
                    }
            if(bmap[y][x]==6)
            {
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0xFF, 0x80, 0x80);
                    }
                if(emap[y][x])
                {
                    cr = 255;
                    cg = 32;
                    cb = 8;
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
            if(bmap[y][x]==7)
            {
                if(emap[y][x])
                {
                    cr = cg = cb = 128;
                    cr += fire_r[y][x];
                    if(cr > 255) cr = 255;
                    fire_r[y][x] = cr;
                    cg += fire_g[y][x];
                    if(cg > 255) cg = 255;
                    fire_g[y][x] = cg;
                    cb += fire_b[y][x];
                    if(cb > 255) cb = 255;
                    fire_b[y][x] = cb;
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(i&j&1)
                            {
                                Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                            }
                }
                else
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(!(i&j&1))
                            {
                                Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                            }
                }
            }
            if(bmap[y][x]==8)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                    {
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0xC0, 0xC0, 0xC0);
                        }
                        else
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                        }
                    }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==11)
            {
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                    {
                        if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0xFF, 0xFF, 0x22);
                        }

                    }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
            if(bmap[y][x]==13)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x57, 0x97, 0x77);
                    }
                }
            }
            if(bmap[y][x]==9)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x3C, 0x3C, 0x3C);
                    }
                }
            }
            if(bmap[y][x]==10)
            {
                for(j=0; j<CELL; j+=2)
                {
                    for(i=(j>>1)&1; i<CELL; i+=2)
                    {
                        Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x57, 0x57, 0x57);
                    }
                }
            }
            if(bmap[y][x]==12)
            {
                if(emap[y][x])
                {
                    for(j=0; j<CELL; j++)
                    {
                        for(i=(j)&1; i<CELL; i++)
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
                        }
                    }
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j)&1; i<CELL; i+=2)
                        {
                            Renderer_DrawPixel(x*CELL+i, y*CELL+j, PIXPACK(0x000000));
                        }
                    }
                }
                else
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j)&1; i<CELL; i+=2)
                        {
                            Renderer_DrawBlob((x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
                        }
                    }
                }
                if(emap[y][x])
                {
                    cr = cg = cb = 16;
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
        }
    
    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
            if(bmap[y][x]==5)
            {
                float lx,ly;
                lx = x*CELL + CELL*0.5f;
                ly = y*CELL + CELL*0.5f;
                for(int t=0; t<1024; t++)
                {
                    nx = (int)(lx+0.5f);
                    ny = (int)(ly+0.5f);
                    if(nx<0 || nx>=XRES || ny<0 || ny>=YRES)
                        break;
                    Renderer_BlendPixel(nx, ny, 255, 255, 255, 64);
                    i = nx/CELL;
                    j = ny/CELL;
                    lx += vx[j][i]*0.125f;
                    ly += vy[j][i]*0.125f;
                    if(bmap[j][i]==5 && i!=x && j!=y)
                        break;
                }
                Graphics_RenderText(x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
            }
}

void Graphics_RenderParticles()
{
    int i, x, y, t, nx, ny, r, s;
    int cr, cg, cb;
    float fr, fg, fb;
    float pt = R_TEMP;
    for(i = 0; i<NPART; i++)
        if(parts[i].type)
        {
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
                        Graphics_RenderText(mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
                    }

                    for(r=-2; r<=1; r++)  //Here I use r variable not as I should, but I think you will excuse me :-p
                    {
                        Renderer_DrawPixel(nx+r, ny-2, ptypes[(int)player[2]].pcolors);
                        Renderer_DrawPixel(nx+r+1, ny+2, ptypes[(int)player[2]].pcolors);
                        Renderer_DrawPixel(nx-2, ny+r+1, ptypes[(int)player[2]].pcolors);
                        Renderer_DrawPixel(nx+2, ny+r, ptypes[(int)player[2]].pcolors);
                    }
                    Renderer_DrawLine(nx, ny+3, player[3], player[4], 255, 255, 255, 255);
                    Renderer_DrawLine(player[3], player[4], player[7], player[8], 255, 255, 255, 255);
                    Renderer_DrawLine(nx, ny+3, player[11], player[12], 255, 255, 255, 255);
                    Renderer_DrawLine(player[11], player[12], player[15], player[16], 255, 255, 255, 255);

                    isplayer = 1;  //It's a secret. Tssss...
                }
                if(t==PT_MWAX&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_DrawPixel(x+nx,y+ny,PIXRGB(224,224,170));
                            else if (y != 0 && x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,224,224,170,20);
                            else
                                Renderer_BlendPixel(x+nx,y+ny,224,224,170,40);
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
                                if (x == 0 && y == 0)
                                    Renderer_BlendPixel(x+nx,y+ny,cr,cg,cb,100);
                                else if (y != 0 || x != 0)
                                    Renderer_BlendPixel(x+nx,y+ny,cr,cg,cb,40);
                            }
                        }
                    } else {
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 255);
                    }

                    if(cmode==4)
                    {
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 223);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 223);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 223);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 223);

                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 112);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 112);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 112);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 112);
                    }
                }
                else if(t==PT_OIL&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,64,64,16,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,64,64,16,40);
                        }
                    }
                }
                else if(t==PT_NEUT)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        Renderer_DrawPixel(nx, ny, ptypes[t].pcolors);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
                    }
                } else if(t==PT_PLUT&&cmode == 6)
                {
                    int tempx;
                    int tempy;
                    cr = 0x40;
                    cg = 0x70;
                    cb = 0x20;
                    Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                    Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                    for(tempx = 2; tempx < 10; tempx++) {
                        for(tempy = 2; tempy < 10; tempy++) {
                            Renderer_BlendPixel(nx+tempx, ny-tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx-tempx, ny+tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx+tempx, ny+tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx-tempx, ny-tempy, cr, cg, cb, 5);
                        }
                    }
                } else if(t==PT_URAN&&cmode == 6)
                {
                    int tempx;
                    int tempy;
                    cr = 0x70;
                    cg = 0x70;
                    cb = 0x20;
                    Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                    Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                    Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                    for(tempx = 2; tempx < 10; tempx++) {
                        for(tempy = 2; tempy < 10; tempy++) {
                            Renderer_BlendPixel(nx+tempx, ny-tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx-tempx, ny+tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx+tempx, ny+tempy, cr, cg, cb, 5);
                            Renderer_BlendPixel(nx-tempx, ny-tempy, cr, cg, cb, 5);
                        }
                    }
                } else if(t==PT_SLTW&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,64,80,240,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,64,80,240,50);
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
                        Renderer_DrawPixel(nx, ny, PIXRGB(cr>255?255:cr,cg>255?255:cg,cb>255?255:cb));
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
                    }
                }
                //Life can be 11 too, so don't just check for 10
                else if(t==PT_SWCH && parts[i].life >= 10)
                {
                    x = nx;
                    y = ny;
                    Renderer_BlendPixel(x,y,17,217,24,255);
                }
                else if(t==PT_LNTG&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,128,160,223,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,128,160,223,50);
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
                                    Renderer_BlendPixel(x+nx,y+ny,100,100,100,30);
                                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                                    Renderer_BlendPixel(x+nx,y+ny,100,100,100,10);
                                if (abs(x)+abs(y) == 2)
                                    Renderer_BlendPixel(x+nx,y+ny,100,100,100,20);
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
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,48,208,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,48,208,50);
                        }
                    }

                } else if(t==PT_DSTW&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,48,208,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,48,208,50);
                        }
                    }
                }
                else if(t==PT_NITR&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,224,16,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,32,224,16,50);
                        }
                    }

                }
                else if(t==PT_LRBD&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,170,170,170,100);
                            else if (y != 0 || x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,170,170,170,50);
                        }
                    }

                }

                else if(t==PT_NBLE&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,235,73,23,100);
                            else if (y != 0 && x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,235,73,23,30);
                            else
                                Renderer_BlendPixel(x+nx,y+ny,235,73,23,50);
                        }
                    }

                }
                else if(t==PT_GAS&&cmode == 6)
                {
                    for(x=-1; x<=1; x++)
                    {
                        for(y=-1; y<=1; y++)
                        {
                            if (x == 0 && y == 0)
                                Renderer_BlendPixel(x+nx,y+ny,255,255,0,180);
                            else if (y != 0 && x != 0)
                                Renderer_BlendPixel(x+nx,y+ny,255,255,0,50);
                            else
                                Renderer_BlendPixel(x+nx,y+ny,255,255,0,80);
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
                                    Renderer_BlendPixel(x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 30);
                                if(abs(x)+abs(y) <=3 && abs(x)+abs(y))
                                    Renderer_BlendPixel(x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 10);
                                if (abs(x)+abs(y) == 2)
                                    Renderer_BlendPixel(x+nx,y+ny, PIXR(ptypes[t].pcolors)/1.6, PIXG(ptypes[t].pcolors)/1.6, PIXB(ptypes[t].pcolors)/1.6, 20);
                            }
                        }
                    }
                }
                else if(t==PT_THDR)
                {
                    if(cmode == 3||cmode==4 || cmode==6)
                    {
                        Renderer_DrawPixel(nx, ny, ptypes[t].pcolors);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                    Renderer_DrawPixel(nx, ny, PIXRGB((int)restrict_flt(0x44 + fr*8, 0, 255), (int)restrict_flt(0x88 + fg*8, 0, 255), (int)restrict_flt(0x44 + fb*8, 0, 255)));
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

                        Renderer_BlendPixel(nx+1, ny, R, G, B, 223);
                        Renderer_BlendPixel(nx-1, ny, R, G, B, 223);
                        Renderer_BlendPixel(nx, ny+1, R, G, B, 223);
                        Renderer_BlendPixel(nx, ny-1, R, G, B, 223);

                        Renderer_BlendPixel(nx+1, ny-1, R, G, B, 112);
                        Renderer_BlendPixel(nx-1, ny-1, R, G, B, 112);
                        Renderer_BlendPixel(nx+1, ny+1, R, G, B, 112);
                        Renderer_BlendPixel(nx-1, ny+1, R, G, B, 112);
                    }
                }
                else if(t==PT_LCRY)
                {
                    uint8 GR = 0x50+(parts[i].life*10);
                    Renderer_DrawPixel(nx, ny, PIXRGB(GR, GR, GR));
                    if(cmode == 4) {
                        Renderer_BlendPixel(nx+1, ny, GR, GR, GR, 223);
                        Renderer_BlendPixel(nx-1, ny, GR, GR, GR, 223);
                        Renderer_BlendPixel(nx, ny+1, GR, GR, GR, 223);
                        Renderer_BlendPixel(nx, ny-1, GR, GR, GR, 223);

                        Renderer_BlendPixel(nx+1, ny-1, GR, GR, GR, 112);
                        Renderer_BlendPixel(nx-1, ny-1, GR, GR, GR, 112);
                        Renderer_BlendPixel(nx+1, ny+1, GR, GR, GR, 112);
                        Renderer_BlendPixel(nx-1, ny+1, GR, GR, GR, 112);
                    }
                }
                else if(t==PT_PCLN)
                {
                    uint8 GR = 0x3B+(parts[i].life*19);
                    Renderer_DrawPixel(nx, ny, PIXRGB(GR, GR, 10));
                    if(cmode == 4) {
                        Renderer_BlendPixel(nx+1, ny, GR, GR, 10, 223);
                        Renderer_BlendPixel(nx-1, ny, GR, GR, 10, 223);
                        Renderer_BlendPixel(nx, ny+1, GR, GR, 10, 223);
                        Renderer_BlendPixel(nx, ny-1, GR, GR, 10, 223);

                        Renderer_BlendPixel(nx+1, ny-1, GR, GR, 10, 112);
                        Renderer_BlendPixel(nx-1, ny-1, GR, GR, 10, 112);
                        Renderer_BlendPixel(nx+1, ny+1, GR, GR, 10, 112);
                        Renderer_BlendPixel(nx-1, ny+1, GR, GR, 10, 112);
                    }
                }
                else if(t==PT_HSWC)
                {
                    uint8 GR = 0x3B+(parts[i].life*19);
                    Renderer_DrawPixel(nx, ny, PIXRGB(GR, 10, 10));
                    if(cmode == 4) {
                        Renderer_BlendPixel(nx+1, ny, GR, 10, 10, 223);
                        Renderer_BlendPixel(nx-1, ny, GR, 10, 10, 223);
                        Renderer_BlendPixel(nx, ny+1, GR, 10, 10, 223);
                        Renderer_BlendPixel(nx, ny-1, GR, 10, 10, 223);

                        Renderer_BlendPixel(nx+1, ny-1, GR, 10, 10, 112);
                        Renderer_BlendPixel(nx-1, ny-1, GR, 10, 10, 112);
                        Renderer_BlendPixel(nx+1, ny+1, GR, 10, 10, 112);
                        Renderer_BlendPixel(nx-1, ny+1, GR, 10, 10, 112);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 192);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                        Renderer_DrawPixel(nx, ny,PIXRGB(cr, cg, cb));
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                        Renderer_BlendPixel(nx, ny, cr, cg, cb, 255);
                        Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 96);
                        Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 32);
                        Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 32);
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
                    Renderer_DrawPixel(nx, ny,PIXRGB(cr, cg, cb));
                    Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 64);
                    Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 64);
                    Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 64);
                    Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 64);
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
                    Renderer_DrawPixel(nx, ny, ptypes[t].pcolors);
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
                    Renderer_DrawPixel(nx, ny, ptypes[t].pcolors);
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
                        Graphics_RenderText(mousex-8-2*(parts[i].life<100)-2*(parts[i].life<10), mousey-12, buff, 255, 255, 255, 255);
                    }

                    for(r=-2; r<=1; r++)
                    {
                        Renderer_DrawPixel(nx+r, ny-2, PIXRGB (R, G, B));
                        Renderer_DrawPixel(nx+r+1, ny+2, PIXRGB (R, G, B));
                        Renderer_DrawPixel(nx-2, ny+r+1, PIXRGB (R, G, B));
                        Renderer_DrawPixel(nx+2, ny+r, PIXRGB (R, G, B));
                    }
                    Renderer_DrawLine(nx, ny+3, player[3], player[4], R, G, B, 255);
                    Renderer_DrawLine(player[3], player[4], player[7], player[8], R, G, B, 255);
                    Renderer_DrawLine(nx, ny+3, player[11], player[12], R, G, B, 255);
                    Renderer_DrawLine(player[11], player[12], player[15], player[16], R, G, B, 255);

                    isplayer = 1;  //It's a secret. Tssss...
                }
                else
                {
                    Renderer_DrawPixel(nx, ny, PIXRGB(R, G, B));
                    //Renderer_BlendPixel(nx+1, ny, R, G, B, 255);
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

                //if(PrimaryBuffer[(ny-1)*YRES+(nx-1)]!=0){
                //    Renderer_BlendPixel(nx, ny-1, R, G, B, 46);
                //}

                Renderer_BlendPixel(nx+1, ny, cr, cg, cb, 223);
                Renderer_BlendPixel(nx-1, ny, cr, cg, cb, 223);
                Renderer_BlendPixel(nx, ny+1, cr, cg, cb, 223);
                Renderer_BlendPixel(nx, ny-1, cr, cg, cb, 223);

                Renderer_BlendPixel(nx+1, ny-1, cr, cg, cb, 112);
                Renderer_BlendPixel(nx-1, ny-1, cr, cg, cb, 112);
                Renderer_BlendPixel(nx+1, ny+1, cr, cg, cb, 112);
                Renderer_BlendPixel(nx-1, ny+1, cr, cg, cb, 112);
            }
        }
}

void Graphics_RenderSigns()
{
    int i, j, x, y, w, h, dx, dy;
    char buff[30];  //Buffer
    for(i=0; i<MAXSIGNS; i++)
        if(signs[i].text[0])
        {
            Interface_GetSignPosition(i, &x, &y, &w, &h);
            Renderer_ClearRectangle(x, y, w, h);
            Renderer_DrawRectangle(x, y, w, h, 192, 192, 192, 255);

            //Displaying special information
            if(strcmp(signs[i].text, "{p}")==0)
            {
                sprintf(buff, "Pressure: %3.2f", pv[signs[i].y/CELL][signs[i].x/CELL]);  //...pressure
                Graphics_RenderText(x+3, y+3, buff, 255, 255, 255, 255);
            }

            if(strcmp(signs[i].text, "{t}")==0)
            {
                if((pmap[signs[i].y][signs[i].x]>>8)>0 && (pmap[signs[i].y][signs[i].x]>>8)<NPART)
                    sprintf(buff, "Temp: %4.2f", parts[pmap[signs[i].y][signs[i].x]>>8].temp-273.15);  //...temperature
                else
                    sprintf(buff, "Temp: 0.00");  //...tempirature
                Graphics_RenderText(x+3, y+3, buff, 255, 255, 255, 255);
            }

            //Usual text
            if(strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}"))
                Graphics_RenderText(x+3, y+3, signs[i].text, 255, 255, 255, 255);
            x = signs[i].x;
            y = signs[i].y;
            dx = 1 - signs[i].ju;
            dy = (signs[i].y > 18) ? -1 : 1;
            for(j=0; j<4; j++)
            {
                Renderer_BlendPixel(x, y, 192, 192, 192, 255);
                x+=dx;
                y+=dy;
            }
        }
}

void Graphics_RenderMenu(int i, int hover)
{
    Renderer_DrawRectangle((XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
    if(hover==i)
    {
        Renderer_FillRectangle((XRES+BARSIZE)-16, (i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16), 14, 14, 255, 255, 255, 255);
        Graphics_RenderText((XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 0, 0, 0, 255);
    }
    else
    {
        Graphics_RenderText((XRES+BARSIZE)-13, (i*16)+YRES+MENUSIZE-14-(SC_TOTAL*16), msections[i].icon, 255, 255, 255, 255);
    }
}

int Graphics_RenderToolsXY(int x, int y, int b, unsigned pc)
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
                        Renderer_DrawPixel(x+i, y+j, pc);
                    }
                    else
                    {
                        Renderer_DrawPixel(x+i, y+j, PIXPACK(0x808080));
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
                        Renderer_DrawPixel(x+i, y+j, pc);
                    }
                }
                for(; i<27; i++)
                {
                    if(i&j&1)
                    {
                        Renderer_DrawPixel(x+i, y+j, pc);
                    }
                }
            }
            break;
        case 24:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            break;
        case 25:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    Renderer_DrawPixel(x+i, y+j, i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000));
                    Graphics_RenderText(x+4, y+3, "\x8D", 255, 255, 255, 255);
                }
            }
            for(i=9; i<26; i++)
            {
                Renderer_DrawPixel(x+i, y+8+(int)(3.9f*cos(i*0.3f)), PIXPACK(0xFFFFFF));
            }
            break;
        case 26:
            for(j=1; j<15; j++)
            {
                for(i=1; i<27; i++)
                {
                    Renderer_DrawPixel(x+i, y+j, i==1||i==26||j==1||j==14 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000));
                }
            }
            Graphics_RenderText(x+9, y+3, "\xA1", 32, 64, 128, 255);
            Graphics_RenderText(x+9, y+3, "\xA0", 255, 255, 255, 255);
            break;
        case 27:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
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
                        Renderer_DrawPixel(x+i, y+j, pc);
                    }
                }
            }
            break;
        case 29:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            break;
        case 30:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<13; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            for(j=1; j<15; j++)
            {
                for(i=14; i<27; i++)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            break;
        case 32:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            break;
        case 33:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
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
                        Renderer_DrawPixel(x+i, y+j, pc);
                    }
                }
            }
            break;
        case 36:
            Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            Graphics_RenderText(x+14-GetTextWidth("AIR")/2, y+4, "AIR", c, c, c, 255);
            break;
        case 37:
            Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            Graphics_RenderText(x+14-GetTextWidth("HEAT")/2, y+4, "HEAT", c, c, c, 255);
            break;
        case 38:
            Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            Graphics_RenderText(x+14-GetTextWidth("COOL")/2, y+4, "COOL", c, c, c, 255);
            break;
        case 39:
            Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
            c = PIXR(pc) + 3*PIXG(pc) + 2*PIXB(pc);
            if(c<544)
            {
                c = 255;
            }
            else
            {
                c = 0;
            }
            Graphics_RenderText(x+14-GetTextWidth("VAC")/2, y+4, "VAC", c, c, c, 255);
            break;
        case 40:
            for(j=1; j<15; j+=2)
            {
                for(i=1+(1&(j>>1)); i<27; i+=2)
                {
                    Renderer_DrawPixel(x+i, y+j, pc);
                }
            }
            break;
        default:
            Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
        }
        if(b==30)
        {
            for(j=4; j<12; j++)
            {
                Renderer_DrawPixel(x+j+6, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x+j+7, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x-j+21, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x-j+22, y+j, PIXPACK(0xFF0000));
            }
        }
    }
    else
    {
        Renderer_FillRectangle(x, y, 27, 15, PIXR(pc), PIXG(pc), PIXB(pc), 255);
        if(b==0)
        {
            for(j=4; j<12; j++)
            {
                Renderer_DrawPixel(x+j+6, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x+j+7, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x-j+21, y+j, PIXPACK(0xFF0000));
                Renderer_DrawPixel(x-j+22, y+j, PIXPACK(0xFF0000));
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
        Graphics_RenderText(x+14-GetTextWidth((char *)ptypes[b].name)/2, y+4, (char *)ptypes[b].name, c, c, c, 255);
    }
    return 26;
}

void Graphics_RenderIcon(int x, int y, char ch, int flag)
{
    char t[2];
    t[0] = ch;
    t[1] = 0;
    if(flag)
    {
        Renderer_FillRectangle(x-1, y-1, 17, 17, 255, 255, 255, 255);
        Graphics_RenderText(x+3, y+2, t, 0, 0, 0, 255);
    }
    else
    {
        Renderer_DrawRectangle(x, y, 15, 15, 255, 255, 255, 255);
        Graphics_RenderText(x+3, y+2, t, 255, 255, 255, 255);
    }
}

int Graphics_RenderThumbnail(void *thumb, int size, int bzip2, int px, int py, int scl)
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
            Renderer_BlendPixel(px+sx, py+sy, r, g, b, 255);
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

void Graphics_RenderCursor(int x, int y, int t, int r)
{
    int i,j,c;
    if(t<PT_NUM||t==SPC_AIR||t==SPC_HEAT||t==SPC_COOL||t==SPC_VACUUM)
    {
        if(r<=0)
            Renderer_XORPixel(x, y);
        else
            for(j=0; j<=r; j++)
                for(i=0; i<=r; i++)
                    if(i*i+j*j<=r*r && ((i+1)*(i+1)+j*j>r*r || i*i+(j+1)*(j+1)>r*r))
                    {
                        Renderer_XORPixel(x+i, y+j);
                        if(j) Renderer_XORPixel(x+i, y-j);
                        if(i) Renderer_XORPixel(x-i, y+j);
                        if(i&&j) Renderer_XORPixel(x-i, y-j);
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
            Renderer_XORPixel(x+i, y);
            Renderer_XORPixel(x+i, y+CELL+c-1);
        }
        for(i=1; i<CELL+c-1; i++)
        {
            Renderer_XORPixel(x, y+i);
            Renderer_XORPixel(x+CELL+c-1, y+i);
        }
    }
}

pixel *Graphics_RescaleImage(pixel *src, int sw, int sh, int *qw, int *qh, int f)
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

pixel *Graphics_PrerenderSave(void *save, int size, int *width, int *height)
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
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF808080);
                break;
            case 2:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF808080);
                break;
            case 3:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(j%2) && !(i%2))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFFC0C0C0);
                break;
            case 4:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF8080FF);
                k++;
                break;
            case 6:
                for(j=0; j<CELL; j+=2)
                    for(i=(j>>1)&1; i<CELL; i+=2)
                        fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFFFF8080);
                break;
            case 7:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(i&j&1))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF808080);
                break;
            case 8:
                for(j=0; j<CELL; j++)
                    for(i=0; i<CELL; i++)
                        if(!(j%2) && !(i%2))
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFFC0C0C0);
                        else
                            fb[(ry+j)*w+(rx+i)] = PIXPACK(0xFF808080);
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
                        fb[(y-2)*w+x+k] = PIXARGB(255, 255, 224, 178);
                        fb[(y+2)*w+x+k+1] = PIXARGB(255, 255, 224, 178);
                        fb[(y+k+1)*w+x-2] = PIXARGB(255, 255, 224, 178);
                        fb[(y+k)*w+x+2] = PIXARGB(255, 255, 224, 178);
                    }
                }
                else
                    fb[y*w+x] = 0xFF000000|ptypes[j].pcolors;
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

// Text related
int Graphics_RenderText(int x, int y, const char *s, int r, int g, int b, int a)
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
            x = Renderer_DrawChar(x, y, *(unsigned char *)s, r, g, b, a);
    }
    return x;
}

int Graphics_RenderWrapText(int x, int y, int w, const char *s, int r, int g, int b, int a)
{
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
            x = Renderer_DrawChar(x, y, *(unsigned char *)s, r, g, b, a);
        }
    }
    return rh;
}

int Graphics_RenderMaxText(int x, int y, int w, char *s, int r, int g, int b, int a)
{
    int i;
    w += x-5;
    for(; *s; s++)
    {
        if(x+font_data[font_ptrs[(int)(*(unsigned char *)s)]]>=w && x+GetTextWidth(s)>=w+5)
            break;
        x = Renderer_DrawChar(x, y, *(unsigned char *)s, r, g, b, a);
    }
    if(*s)
        for(i=0; i<3; i++)
            x = Renderer_DrawChar(x, y, '.', r, g, b, a);
    return x;
}

int GetTextWidth(char *s)
{
    int x = 0;
    for(; *s; s++)
        x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
    return x-1;
}

int GetTextNWidth(char *s, int n)
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

void GetTextNPos(char *s, int n, int w, int *cx, int *cy)
{
    int x = 0;
    int y = 0;
    //TODO: Implement Textnheight for wrapped text
    for(; *s; s++)
    {
        if(!n){
            break;
        }
        x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
        if(x>=w) {
            x = 0;
            y += FONT_H+2;
        }
        n--;
    }
    *cx = x-1;
    *cy = y;
}

int GetTextWidthX(char *s, int w)
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

int GetTextPosXY(char *s, int width, int w, int h)
{
    int x=0,y=0,n=0,cw;
    for(; *s; s++)
    {
        cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
        if(x+(cw/2) >= w && y+6 >= h)
            break;
        x += cw;
        if(x>=width) {
            x = 0;
            y += FONT_H+2;
        }
        n++;
    }
    return n;
}
