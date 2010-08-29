#include <math.h>
#include "air.h"
#include "powder.h"
#include "defines.h"
float kernel[9];

float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];

float cb_vx[YRES/CELL][XRES/CELL], cb_ovx[YRES/CELL][XRES/CELL];
float cb_vy[YRES/CELL][XRES/CELL], cb_ovy[YRES/CELL][XRES/CELL];
float cb_pv[YRES/CELL][XRES/CELL], cb_opv[YRES/CELL][XRES/CELL];

float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];

void make_kernel(void)
{
    int i, j;
    float s = 0.0f;
    for(j=-1; j<2; j++)
        for(i=-1; i<2; i++)
        {
            kernel[(i+1)+3*(j+1)] = expf(-2.0f*(i*i+j*j));
            s += kernel[(i+1)+3*(j+1)];
        }
    s = 1.0f / s;
    for(j=-1; j<2; j++)
        for(i=-1; i<2; i++)
            kernel[(i+1)+3*(j+1)] *= s;
}
void update_air(void)
{
    int x, y, i, j;
    float dp, dx, dy, f, tx, ty;

    for(y=1; y<YRES/CELL; y++)
        for(x=1; x<XRES/CELL; x++)
        {
            dp = 0.0f;
            dp += vx[y][x-1] - vx[y][x];
            dp += vy[y-1][x] - vy[y][x];
            pv[y][x] *= PLOSS;
            pv[y][x] += dp*TSTEPP;
        }

    for(y=0; y<YRES/CELL-1; y++)
        for(x=0; x<XRES/CELL-1; x++)
        {
            dx = dy = 0.0f;
            dx += pv[y][x] - pv[y][x+1];
            dy += pv[y][x] - pv[y+1][x];
            vx[y][x] *= VLOSS;
            vy[y][x] *= VLOSS;
            vx[y][x] += dx*TSTEPV;
            vy[y][x] += dy*TSTEPV;
            if(bmap[y][x]==1 || bmap[y][x+1]==1 ||
                    bmap[y][x]==8 || bmap[y][x+1]==8 ||
                    (bmap[y][x]==7 && !emap[y][x]) ||
                    (bmap[y][x+1]==7 && !emap[y][x+1]))
                vx[y][x] = 0;
            if(bmap[y][x]==1 || bmap[y+1][x]==1 ||
                    bmap[y][x]==8 || bmap[y+1][x]==8 ||
                    (bmap[y][x]==7 && !emap[y][x]) ||
                    (bmap[y+1][x]==7 && !emap[y+1][x]))
                vy[y][x] = 0;
        }

    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
        {
            dx = 0.0f;
            dy = 0.0f;
            dp = 0.0f;
            for(j=-1; j<2; j++)
                for(i=-1; i<2; i++)
                    if(y+j>0 && y+j<YRES/CELL-1 &&
                            x+i>0 && x+i<XRES/CELL-1 &&
                            bmap[y+j][x+i]!=1 &&
                            bmap[y+j][x+i]!=8 &&
                            (bmap[y+j][x+i]!=7 || emap[y+j][x+i]))
                    {
                        f = kernel[i+1+(j+1)*3];
                        dx += vx[y+j][x+i]*f;
                        dy += vy[y+j][x+i]*f;
                        dp += pv[y+j][x+i]*f;
                    }
                    else
                    {
                        f = kernel[i+1+(j+1)*3];
                        dx += vx[y][x]*f;
                        dy += vy[y][x]*f;
                        dp += pv[y][x]*f;
                    }

            tx = x - dx*0.7f;
            ty = y - dy*0.7f;
            i = (int)tx;
            j = (int)ty;
            tx -= i;
            ty -= j;
            if(i>=2 && i<XRES/CELL-3 &&
                    j>=2 && j<YRES/CELL-3)
            {
                dx *= 1.0f - VADV;
                dy *= 1.0f - VADV;

                dx += VADV*(1.0f-tx)*(1.0f-ty)*vx[j][i];
                dy += VADV*(1.0f-tx)*(1.0f-ty)*vy[j][i];

                dx += VADV*tx*(1.0f-ty)*vx[j][i+1];
                dy += VADV*tx*(1.0f-ty)*vy[j][i+1];

                dx += VADV*(1.0f-tx)*ty*vx[j+1][i];
                dy += VADV*(1.0f-tx)*ty*vy[j+1][i];

                dx += VADV*tx*ty*vx[j+1][i+1];
                dy += VADV*tx*ty*vy[j+1][i+1];
            }

            if(bmap[y][x] == 4)
            {
                dx += fvx[y][x];
                dy += fvy[y][x];
            }

            if(dp > 256.0f) dp = 256.0f;
            if(dp < -256.0f) dp = -256.0f;
            if(dx > 256.0f) dx = 256.0f;
            if(dx < -256.0f) dx = -256.0f;
            if(dy > 256.0f) dy = 256.0f;
            if(dy < -256.0f) dy = -256.0f;

            ovx[y][x] = dx;
            ovy[y][x] = dy;
            opv[y][x] = dp;
        }
    memcpy(vx, ovx, sizeof(vx));
    memcpy(vy, ovy, sizeof(vy));
    memcpy(pv, opv, sizeof(pv));
}