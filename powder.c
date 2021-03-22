/**
 * Powder Toy - Main source
 *
 * Powder Toy is the legal property of Stanislaw Skowronek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <bzlib.h>
#include <time.h>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "http.h"
#include "md5.h"
#include "update.h"

#define SAVE_VERSION 24
#define MINOR_VERSION 9

#define SERVER "powder.unaligned.org"

#undef PLOSS

#define XRES	512
#define YRES	384

#define ZSIZE	16
#define ZFACTOR	8

#define CELL    4
#define ISTP    (CELL/2)
#define CFDS	(4.0f/CELL)

#ifdef PIX16
#define PIXELSIZE 2
typedef unsigned short pixel;
#define PIXPACK(x) ((((x)>>8)&0xF800)|(((x)>>5)&0x07E0)|(((x)>>3)&0x001F))
#define PIXRGB(r,g,b) ((((r)<<8)&0xF800)|(((g)<<3)&0x07E0)|(((b)>>3)&0x001F))
#define PIXR(x) (((x)>>8)&0xF8)
#define PIXG(x) (((x)>>3)&0xFC)
#define PIXB(x) (((x)<<3)&0xF8)
#else
#define PIXELSIZE 4
typedef unsigned int pixel;
#ifdef PIX32BGR
#define PIXPACK(x) ((((x)>>16)&0x0000FF)|((x)&0x00FF00)|(((x)<<16)&0xFF0000))
#define PIXRGB(r,g,b) (((b)<<16)|((g)<<8)|(r))
#define PIXR(x) ((x)&0xFF)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)>>16)
#else
#define PIXPACK(x) (x)
#define PIXRGB(r,g,b) (((r)<<16)|((g)<<8)|(b))
#define PIXR(x) ((x)>>16)
#define PIXG(x) (((x)>>8)&0xFF)
#define PIXB(x) ((x)&0xFF)
#endif
#endif

char *it_msg =
    "\brThe Powder Toy\n"
    "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\n"
    "\bwMaterials on the bottom right are walls, materials on the bottom left are particles.\n"
    "\n"
    "\bgPick your material from the bottom bar using mouse left/right buttons.\n"
    "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
    "Shift+drag will create straight lines of particles.\n"
    "Ctrl+drag will result in filled rectangles.\n"
    "Ctrl+Shift+click will flood-fill a closed area.\n"
    "\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around.\n"
    "Use 'S' to save parts of the window as 'stamps'.\n"
    "'L' will load the most recent stamp, 'K' shows a library of stamps you saved.\n"
    "\n"
    "\brhttp://powder.unaligned.org/\n"
    "\bbirc.unaligned.org #wtf\n"
    "\n"
    "\bg(c) 2008-9 Stanislaw K Skowronek\n"
    "\n"
    "\bgSpecial thanks to Brian Ledbetter for maintaining ports & server development."
    "\nThanks to CW for hosting the server."
#ifdef WIN32
    "\nThanks to Akuryo for Windows icons."
#endif
    ;

char *old_ver_msg = "A new version is available - click here!";

int sys_pause = 0;

int amd = 0;

unsigned char fire_r[YRES/CELL][XRES/CELL];
unsigned char fire_g[YRES/CELL][XRES/CELL];
unsigned char fire_b[YRES/CELL][XRES/CELL];

#define MAXSIGNS 16

struct sign {
    int x,y,ju;
    char text[256];
} signs[MAXSIGNS];

/***********************************************************
 *                   AIR FLOW SIMULATOR                    *
 ***********************************************************/

unsigned char bmap[YRES/CELL][XRES/CELL];
unsigned char emap[YRES/CELL][XRES/CELL];
unsigned cmode = 3;

float vx[YRES/CELL][XRES/CELL], ovx[YRES/CELL][XRES/CELL];
float vy[YRES/CELL][XRES/CELL], ovy[YRES/CELL][XRES/CELL];
float pv[YRES/CELL][XRES/CELL], opv[YRES/CELL][XRES/CELL];
float fvx[YRES/CELL][XRES/CELL], fvy[YRES/CELL][XRES/CELL];
#define TSTEPP 0.3f
#define TSTEPV 0.4f
#define VADV 0.3f
#define VLOSS 0.999f
#define PLOSS 0.9999f

float kernel[9];
void make_kernel(void)
{
    int i, j;
    float s = 0.0f;
    for(j=-1; j<2; j++)
	for(i=-1; i<2; i++) {
	    kernel[(i+1)+3*(j+1)] = exp(-2.0f*(i*i+j*j));
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
	for(x=1; x<XRES/CELL; x++) {
	    dp = 0.0f;
	    dp += vx[y][x-1] - vx[y][x];
	    dp += vy[y-1][x] - vy[y][x];
	    pv[y][x] *= PLOSS;
	    pv[y][x] += dp*TSTEPP;
	}

    for(y=0; y<YRES/CELL-1; y++)
	for(x=0; x<XRES/CELL-1; x++) {
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
	for(x=0; x<XRES/CELL; x++) {
	    dx = 0.0f;
	    dy = 0.0f;
	    dp = 0.0f;
	    for(j=-1; j<2; j++)
		for(i=-1; i<2; i++)
		    if(y+j>0 && y+j<YRES/CELL-1 &&
		       x+i>0 && x+i<XRES/CELL-1 &&
		       bmap[y+j][x+i]!=1 &&
		       bmap[y+j][x+i]!=8 &&
		       (bmap[y+j][x+i]!=7 || emap[y+j][x+i])) {
			f = kernel[i+1+(j+1)*3];
		        dx += vx[y+j][x+i]*f;
		        dy += vy[y+j][x+i]*f;
			dp += pv[y+j][x+i]*f;
		    } else {
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
	       j>=2 && j<YRES/CELL-3) {
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

	    if(bmap[y][x] == 4){
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

unsigned clamp_flt(float f, float min, float max)
{
    if(f<min)
	return 0;
    if(f>max)
	return 255;
    return (int)(255.0f*(f-min)/(max-min));
}

void draw_air(pixel *vid)
{
    int x, y, i, j;
    pixel c;

    if(cmode == 2)
	return;

    for(y=0; y<YRES/CELL; y++)
	for(x=0; x<XRES/CELL; x++) {
	    if(cmode) {
		if(pv[y][x] > 0.0f)
		    c  = PIXRGB(clamp_flt(pv[y][x], 0.0f, 8.0f), 0, 0);
		else
		    c  = PIXRGB(0, 0, clamp_flt(-pv[y][x], 0.0f, 8.0f));
	    } else
		c  = PIXRGB(clamp_flt(fabs(vx[y][x]), 0.0f, 8.0f),
			    clamp_flt(pv[y][x], 0.0f, 8.0f),
			    clamp_flt(fabs(vy[y][x]), 0.0f, 8.0f));
	    for(j=0; j<CELL; j++)
		for(i=0; i<CELL; i++)
		    vid[(x*CELL+i) + (y*CELL+j)*XRES] = c;
	}
}

/***********************************************************
 *                   PARTICLE SIMULATOR                    *
 ***********************************************************/

#define NPART 65536

#define PT_NONE	0
#define PT_DUST	1
#define PT_WATR	2
#define PT_OILL 3
#define PT_FIRE 4
#define PT_METL 5
#define PT_LAVA 6
#define PT_GUNP	7
#define PT_NITR	8
#define PT_CLNE 9
#define PT_GASS 10
#define PT_PLEX 11
#define PT_DFRM 12
#define PT_ICEI 13
#define PT_WIRE 14
#define PT_SPRK 15
#define PT_SNOW 16
#define PT_WOOD 17
#define PT_NEUT 18
#define PT_PLUT 19
#define PT_NUM  20

char *names[] = {
    "",
    "DUST",
    "WATR",
    "OIL",
    "FIRE",
    "STNE",
    "LAVA",
    "GUN",
    "NITR",
    "CLNE",
    "GAS",
    "C-4",
    "GOO",
    "ICE",
    "METL",
    "SPRK",
    "SNOW",
    "WOOD",
    "NEUT",
    "PLUT",
};
char *descs[] = {
    "Erases particles.",
    "Very light dust. Flammable.",
    "Liquid. Conducts electricity. Freezes. Extinguishes fires.",
    "Liquid. Flammable.",
    "Ignites flammable materials. Heats air.",
    "Heavy particles. Meltable.",
    "Heavy liquid. Ignites flammable materials. Solidifies when cold.",
    "Light dust. Explosive.",
    "Liquid. Pressure sensitive explosive.",
    "Solid. Duplicates any particles it touches.",
    "Gas. Diffuses. Flammable. Liquifies under pressure.",
    "Solid. Pressure sensitive explosive.",
    "Solid. Deforms and disappears under pressure.",
    "Solid. Freezes water. Crushes under pressure. Cools down air.",
    "Solid. Conducts electricity. Meltable.",
    "Electricity. Conducted by metal and water.",
    "Light particles.",
    "Solid. Flammable.",
    "Neutrons. Interact with matter in odd ways.",
    "Heavy particles. Fissile. Generates neutrons under pressure.",
};
pixel pcolors[] = {
    PIXPACK(0x000000),
    PIXPACK(0xFFE0A0),
    PIXPACK(0x2030D0),
    PIXPACK(0x404010),
    PIXPACK(0xFF1000),
    PIXPACK(0xA0A0A0),
    PIXPACK(0xE05010),
    PIXPACK(0xC0C0D0),
    PIXPACK(0x20E010),
    PIXPACK(0xFFD010),
    PIXPACK(0xE0FF20),
    PIXPACK(0xD080E0),
    PIXPACK(0x804000),
    PIXPACK(0xA0C0FF),
    PIXPACK(0x404060),
    PIXPACK(0xFFFF80),
    PIXPACK(0xC0E0FF),
    PIXPACK(0xC0A040),
    PIXPACK(0x20E0FF),
    PIXPACK(0x407020),
};
float advection[] = {
    0.0f,
    0.7f,
    0.6f,
    0.6f,
    0.9f,
    0.4f,
    0.3f,
    0.7f,
    0.5f,
    0.0f,
    1.0f,
    0.0f,
    0.1f,
    0.0f,
    0.0f,
    0.0f,
    0.7f,
    0.0f,
    0.0f,
    0.4f,
};
float airdrag[] = {
    0.00f * CFDS,
    0.02f * CFDS,
    0.01f * CFDS,
    0.01f * CFDS,
    0.04f * CFDS,
    0.04f * CFDS,
    0.02f * CFDS,
    0.02f * CFDS,
    0.02f * CFDS,
    0.00f * CFDS,
    0.01f * CFDS,
    0.00f * CFDS,
    0.00f * CFDS,
    0.00f * CFDS,
    0.00f * CFDS,
    0.00f * CFDS,
    0.01f * CFDS,
    0.00f * CFDS,
    0.00f * CFDS,
    0.01f * CFDS,
};
float airloss[] = {
    1.00f,
    0.96f,
    0.98f,
    0.98f,
    0.97f,
    0.94f,
    0.95f,
    0.94f,
    0.92f,
    0.90f,
    0.99f,
    0.90f,
    0.97f,
    0.90f,
    0.90f,
    0.90f,
    0.96f,
    0.90f,
    1.00f,
    0.99f,
};
float loss[] = {
    0.00f,
    0.80f,
    0.95f,
    0.95f,
    0.20f,
    0.95f,
    0.80f,
    0.80f,
    0.97f,
    0.00f,
    0.30f,
    0.00f,
    0.50f,
    0.00f,
    0.00f,
    0.00f,
    0.90f,
    0.00f,
    1.00f,
    0.95f,
};
float collision[] = {
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    -0.1f,
    0.0f,
    -0.1f,
    0.0f,
    0.0f,
    -0.1f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    -0.1f,
    0.0f,
    -0.99f,
    0.0f,
};
float gravity[] = {
    0.0f,
    0.1f,
    0.1f,
    0.1f,
    -0.1f,
    0.3f,
    0.15f,
    0.1f,
    0.2f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.05f,
    0.0f,
    0.0f,
    0.4f,
};
float diffusion[] = {
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.75f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.00f,
    0.01f,
    0.00f,
    0.01f,
    0.00f,
};
float hotair[] = {
    0.000f   * CFDS,
    0.000f   * CFDS,
    0.000f   * CFDS,
    0.000f   * CFDS,
    0.001f   * CFDS,
    0.000f   * CFDS,
    0.0003f  * CFDS,
    0.000f   * CFDS,
    0.000f   * CFDS,
    0.000f   * CFDS,
    0.001f   * CFDS,
    0.000f   * CFDS,
    0.000f   * CFDS,
    -0.0003f * CFDS,
    0.000f   * CFDS,
    0.001f   * CFDS,
    -0.00005f* CFDS,
    0.000f   * CFDS,
    0.002f   * CFDS,
    0.000f   * CFDS,
};
int falldown[] = {
    0,
    1,
    2,
    2,
    1,
    1,
    2,
    1,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    1,
};
int flammable[] = {
    0,
    10,
    0,
    20,
    0,
    1,
    0,
    600,
    1000,
    0,
    600,
    1000,
    0,
    0,
    0,
    0,
    0,
    20,
    0,
    0,
};
int explosive[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    2,
    0,
    0,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
int meltable[] = {
    0,
    0,
    0,
    0,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    50,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
};

#define FLAG_STAGNANT	1
typedef struct {
    int type;
    int life, ctype;
    float x, y, vx, vy;
    int flags;
} particle;
particle *parts;

int pfree;

unsigned pmap[YRES][XRES];

int try_move(int i, int x, int y, int nx, int ny)
{
    unsigned r;

    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES)
	return 0;
    if(x==nx && y==ny)
	return 1;
    r = pmap[ny][nx];
    if(r && (r>>8)<NPART)
	r = (r&~0xFF) | parts[r>>8].type;

    if(falldown[parts[i].type]!=2 && bmap[ny/CELL][nx/CELL]==3)
	return 0;
    if(parts[i].type==PT_NEUT && bmap[ny/CELL][nx/CELL]==7 && !emap[ny/CELL][nx/CELL])
	return 0;
    if(r && (r>>8)<NPART && falldown[r&0xFF]!=2 && bmap[y/CELL][x/CELL]==3)
	return 0;

    if(r &&
       !(parts[i].type==PT_METL && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_DUST || (r&0xFF)==PT_SNOW || (r&0xFF)==PT_GASS || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_NITR || (r&0xFF)==PT_LAVA)) &&
       !(parts[i].type==PT_PLUT && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_DUST || (r&0xFF)==PT_SNOW || (r&0xFF)==PT_GASS || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_NITR || (r&0xFF)==PT_LAVA)) &&
       !(parts[i].type==PT_LAVA && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_DUST || (r&0xFF)==PT_SNOW || (r&0xFF)==PT_GASS || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_NITR)) &&
       !(parts[i].type==PT_DUST && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_GASS || (r&0xFF)==PT_NITR)) &&
       !(parts[i].type==PT_SNOW && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_GASS || (r&0xFF)==PT_NITR)) &&
       !(parts[i].type==PT_GUNP && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_OILL || (r&0xFF)==PT_GASS || (r&0xFF)==PT_NITR)) &&
       !(parts[i].type==PT_WATR && ((r&0xFF)==PT_OILL || (r&0xFF)==PT_NITR || (r&0xFF)==PT_GASS)) &&
       !(parts[i].type==PT_NITR && ((r&0xFF)==PT_OILL || (r&0xFF)==PT_GASS)) &&
       !(parts[i].type==PT_OILL && ((r&0xFF)==PT_GASS || (r&0xFF)==PT_FIRE)) &&
       !(parts[i].type==PT_LAVA && (r&0xFF)==PT_FIRE) &&
       !(parts[i].type==PT_FIRE && (r&0xFF)==PT_LAVA) &&
       !(parts[i].type==PT_NEUT && (r&0xFF)!=PT_WIRE && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_LAVA && (r&0xFF)!=PT_METL))
	return 0;

    pmap[ny][nx] = (i<<8)|parts[i].type;
    pmap[y][x] = r;
    if(r && (r>>8)<NPART) {
	r >>= 8;
	parts[r].x += x-nx;
	parts[r].y += y-ny;
    }

    return 1;
}

void kill_part(int i)
{
    int x, y;
    parts[i].type = PT_NONE;

    x = (int)(parts[i].x+0.5f);
    y = (int)(parts[i].y+0.5f);

    if(x>=0 && y>=0 && x<XRES && y<YRES)
	pmap[y][x] = 0;

    parts[i].life = pfree;
    pfree = i;
}

int create_part(int p, int x, int y, int t)
{
    int i;

    if(x<0 || y<0 || x>=XRES || y>=YRES)
	return -1;

    if(t==PT_SPRK) {
	if((pmap[y][x]&0xFF)!=PT_WIRE &&
	   (pmap[y][x]&0xFF)!=PT_WATR)
	    return -1;
	parts[pmap[y][x]>>8].type = PT_SPRK;
	parts[pmap[y][x]>>8].life = 4;
	parts[pmap[y][x]>>8].ctype = pmap[y][x]&0xFF;
	pmap[y][x] = (pmap[y][x]&~0xFF) | PT_SPRK;
	return pmap[y][x]>>8;
    }

    if(p==-1) {
	if(pmap[y][x])
	    return -1;
	if(pfree == -1)
	    return -1;
	i = pfree;
	pfree = parts[i].life;
    } else
	i = p;

    parts[i].x = x;
    parts[i].y = y;
    parts[i].type = t;
    parts[i].vx = 0;
    parts[i].vy = 0;
    parts[i].life = 0;
    parts[i].ctype = 0;
    if(t==PT_FIRE)
	parts[i].life = rand()%50+120;
    if(t==PT_LAVA)
	parts[i].life = rand()%120+240;
    if(t==PT_NEUT) {
	float r = (rand()%128+128)/127.0f;
	float a = (rand()%360)*3.14159f/180.0f;
	parts[i].life = rand()%480+480;
	parts[i].vx = r*cos(a);
	parts[i].vy = r*sin(a);
    }

    pmap[y][x] = t|(i<<8);

    return i;
}

void delete_part(int x, int y)
{
    unsigned i;

    if(x<0 || y<0 || x>=XRES || y>=YRES)
	return;
    i = pmap[y][x];
    if(!i || (i>>8)>=NPART)
	return;

    kill_part(i>>8);
    pmap[y][x] = 0;	// just in case
}

void blendpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
{
    pixel t;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
	return;
    if(a!=255) {
	t = vid[y*XRES+x];
	r = (a*r + (255-a)*PIXR(t)) >> 8;
	g = (a*g + (255-a)*PIXG(t)) >> 8;
	b = (a*b + (255-a)*PIXB(t)) >> 8;
    }
    vid[y*XRES+x] = PIXRGB(r,g,b);
}

inline void addpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
{
    pixel t;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
	return;
    t = vid[y*XRES+x];
    r = (a*r + 255*PIXR(t)) >> 8;
    g = (a*g + 255*PIXG(t)) >> 8;
    b = (a*b + 255*PIXB(t)) >> 8;
    if(r>255)
	r = 255;
    if(g>255)
	g = 255;
    if(b>255)
        b = 255;
    vid[y*XRES+x] = PIXRGB(r,g,b);
}

int drawtext(pixel *vid, int x, int y, char *s, int r, int g, int b, int a);

int is_wire(int x, int y)
{
    return bmap[y][x]==6 || bmap[y][x]==7 || bmap[y][x]==3 || bmap[y][x]==8;
}
int is_wire_off(int x, int y)
{
    return (bmap[y][x]==6 || bmap[y][x]==7 || bmap[y][x]==3 || bmap[y][x]==8) && emap[y][x]<8;
}

void set_emap(int x, int y)
{
    int x1, x2;

    if(!is_wire_off(x, y))
	return;

    // go left as far as possible
    x1 = x2 = x;
    while(x1>0) {
	if(!is_wire_off(x1-1, y))
	    break;
	x1--;
    }
    while(x2<XRES/CELL-1) {
	if(!is_wire_off(x2+1, y))
	    break;
	x2++;
    }

    // fill span
    for(x=x1; x<=x2; x++)
	emap[y][x] = 16;

    // fill children

    if(y>1 && x1==x2 &&
       is_wire(x1-1, y-1) && is_wire(x1, y-1) && is_wire(x1+1, y-1) &&
       !is_wire(x1-1, y-2) && is_wire(x1, y-2) && !is_wire(x1+1, y-2))
	set_emap(x1, y-2);
    else if(y>0)
	for(x=x1; x<=x2; x++)
	    if(is_wire_off(x, y-1)) {
		if(x==x1 || x==x2 || y>=YRES/CELL-1 ||
		   is_wire(x-1, y-1) || is_wire(x+1, y-1) ||
		   is_wire(x-1, y+1) || !is_wire(x, y+1) || is_wire(x+1, y+1))
		    set_emap(x, y-1);
	    }

    if(y<YRES/CELL-2 && x1==x2 &&
       is_wire(x1-1, y+1) && is_wire(x1, y+1) && is_wire(x1+1, y+1) &&
       !is_wire(x1-1, y+2) && is_wire(x1, y+2) && !is_wire(x1+1, y+2))
	set_emap(x1, y+2);
    else if(y<YRES/CELL-1)
	for(x=x1; x<=x2; x++)
	    if(is_wire_off(x, y+1)) {
		if(x==x1 || x==x2 || y<0 ||
		   is_wire(x-1, y+1) || is_wire(x+1, y+1) ||
		   is_wire(x-1, y-1) || !is_wire(x, y-1) || is_wire(x+1, y-1))
		    set_emap(x, y+1);
	    }
}

void update_particles(pixel *vid)
{
    int i, j, x, y, t, nx, ny, r, a, cr,cg,cb, s, l = -1, rt, fe, nt, lpv;
    float mv, dx, dy, ix, iy, lx, ly;

    memset(pmap, 0, sizeof(pmap));
    r = rand()%2;
    for(j=0; j<NPART; j++) {
	i = r ? (NPART-1-j) : j;
	if(parts[i].type) {
	    t = parts[i].type;
	    x = (int)(parts[i].x+0.5f);
	    y = (int)(parts[i].y+0.5f);
	    if(x>=0 && y>=0 && x<XRES && y<YRES)
		pmap[y][x] = t|(i<<8);
	} else {
	    parts[i].life = l;
	    l = i;
	}
    }
    pfree=l;

    for(y=0; y<YRES/CELL; y++)
	for(x=0; x<XRES/CELL; x++) {
	    if(bmap[y][x]==1)
		for(j=0; j<CELL; j++)
		    for(i=0; i<CELL; i++) {
			pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
			vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x808080);
		    }
	    if(bmap[y][x]==2)
		for(j=0; j<CELL; j+=2)
		    for(i=(j>>1)&1; i<CELL; i+=2)
			vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x808080);
	    if(bmap[y][x]==3) {
		for(j=0; j<CELL; j++)
		    for(i=0; i<CELL; i++)
			if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
			    vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0xC0C0C0);
		if(emap[y][x]) {
		    cr = cg = cb = 16;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		}
	    }
	    if(bmap[y][x]==4)
		for(j=0; j<CELL; j+=2)
		    for(i=(j>>1)&1; i<CELL; i+=2)
			vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x8080FF);
	    if(bmap[y][x]==6) {
		for(j=0; j<CELL; j+=2)
		    for(i=(j>>1)&1; i<CELL; i+=2)
			vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0xFF8080);
		if(emap[y][x]) {
		    cr = 255;
		    cg = 32;
		    cb = 8;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		}
	    }
	    if(bmap[y][x]==7) {
		if(emap[y][x]) {
		    cr = cg = cb = 128;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		    for(j=0; j<CELL; j++)
			for(i=0; i<CELL; i++)
			    if(i&j&1)
				vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x808080);
		} else {
		    for(j=0; j<CELL; j++)
			for(i=0; i<CELL; i++)
			    pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
		    for(j=0; j<CELL; j++)
			for(i=0; i<CELL; i++)
			    if(!(i&j&1))
				vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x808080);
		}
	    }
	    if(bmap[y][x]==8) {
		for(j=0; j<CELL; j++)
		    for(i=0; i<CELL; i++) {
			pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
			if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
			    vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0xC0C0C0);
			else
			    vid[(y*CELL+j)*XRES+(x*CELL+i)] = PIXPACK(0x808080);
		    }
		if(emap[y][x]) {
		    cr = cg = cb = 16;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		}
	    }
	    if(emap[y][x] && !sys_pause)
		emap[y][x] --;
	}

    for(i=0; i<NPART; i++)
	if(parts[i].type) {

	    lx = parts[i].x;
	    ly = parts[i].y;
	    t = parts[i].type;

	    if(sys_pause)
		goto justdraw;

	    if(parts[i].life) {
		parts[i].life--;
		if(parts[i].life<=0 && t!=PT_WIRE && t!=PT_WATR && t!=PT_SPRK && t!=PT_LAVA) {
		    kill_part(i);
		    continue;
		}
		if(parts[i].life<=0 && t==PT_SPRK) {
		    t = parts[i].ctype;
		    if(!t)
			t = PT_WIRE;
		    parts[i].type = t;
		    parts[i].life = 4;
		    if(t == PT_WATR)
			parts[i].life = 64;
		}
	    }

	    x = (int)(parts[i].x+0.5f);
	    y = (int)(parts[i].y+0.5f);

	    if(x<0 || y<0 || x>=XRES || y>=YRES ||
	       bmap[y/CELL][x/CELL]==1 ||
	       bmap[y/CELL][x/CELL]==8 ||
	       (bmap[y/CELL][x/CELL]==2) ||
	       (bmap[y/CELL][x/CELL]==3 && falldown[t]!=2) ||
	       (bmap[y/CELL][x/CELL]==6 && (t==PT_WIRE || t==PT_SPRK)) ||
	       (bmap[y/CELL][x/CELL]==7 && !emap[y/CELL][x/CELL])) {
		kill_part(i);
		continue;
	    }

	    vx[y/CELL][x/CELL] *= airloss[t];
	    vy[y/CELL][x/CELL] *= airloss[t];
	    vx[y/CELL][x/CELL] += airdrag[t]*parts[i].vx;
	    vy[y/CELL][x/CELL] += airdrag[t]*parts[i].vy;
	    if(t==PT_GASS) {
		if(pv[y/CELL][x/CELL]<3.5f)
		    pv[y/CELL][x/CELL] += hotair[t]*(3.5f-pv[y/CELL][x/CELL]);
		if(y+CELL<YRES && pv[y/CELL+1][x/CELL]<3.5f)
		    pv[y/CELL+1][x/CELL] += hotair[t]*(3.5f-pv[y/CELL+1][x/CELL]);
		if(x+CELL<XRES) {
		    pv[y/CELL][x/CELL+1] += hotair[t]*(3.5f-pv[y/CELL][x/CELL+1]);
		    if(y+CELL<YRES)
			pv[y/CELL+1][x/CELL+1] += hotair[t]*(3.5f-pv[y/CELL+1][x/CELL+1]);
		}
	    } else {
		pv[y/CELL][x/CELL] += hotair[t];
		if(y+CELL<YRES)
		    pv[y/CELL+1][x/CELL] += hotair[t];
		if(x+CELL<XRES) {
		    pv[y/CELL][x/CELL+1] += hotair[t];
		    if(y+CELL<YRES)
			pv[y/CELL+1][x/CELL+1] += hotair[t];
		}
	    }

	    if((explosive[t]&2) && pv[y/CELL][x/CELL]>2.5f) {
		parts[i].life = rand()%80+180;
		parts[i].type = PT_FIRE;
		pv[y/CELL][x/CELL] += 0.25f * CFDS;
		t = PT_FIRE;
	    }

	    parts[i].vx *= loss[t];
	    parts[i].vy *= loss[t];

	    if(t==PT_DFRM && !parts[i].life) {
		if(pv[y/CELL][x/CELL]>1.0f) {
		    parts[i].vx += advection[t]*vx[y/CELL][x/CELL];
		    parts[i].vy += advection[t]*vy[y/CELL][x/CELL];
		    parts[i].life = rand()%80+300;
		}
	    } else {
		parts[i].vx += advection[t]*vx[y/CELL][x/CELL];
		parts[i].vy += advection[t]*vy[y/CELL][x/CELL] + gravity[t];
	    }

	    if(diffusion[t]) {
		parts[i].vx += diffusion[t]*(rand()/(0.5f*RAND_MAX)-1.0f);
		parts[i].vy += diffusion[t]*(rand()/(0.5f*RAND_MAX)-1.0f);
	    }

	    // interpolator
	    mv = fmaxf(fabs(parts[i].vx), fabs(parts[i].vy));
	    if(mv < ISTP) {
		parts[i].x += parts[i].vx;
		parts[i].y += parts[i].vy;
		ix = parts[i].x;
		iy = parts[i].y;
	    } else {
		dx = parts[i].vx*ISTP/mv;
		dy = parts[i].vy*ISTP/mv;
		ix = parts[i].x;
		iy = parts[i].y;
		while(1) {
		    mv -= ISTP;
		    if(mv <= 0.0f) {
			// nothing found
			parts[i].x += parts[i].vx;
			parts[i].y += parts[i].vy;
			ix = parts[i].x;
			iy = parts[i].y;
			break;
		    }
		    ix += dx;
		    iy += dy;
		    nx = (int)(ix+0.5f);
		    ny = (int)(iy+0.5f);
		    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES || pmap[ny][nx] || (bmap[ny/CELL][nx/CELL] && bmap[ny/CELL][nx/CELL]!=5)) {
			parts[i].x = ix;
			parts[i].y = iy;
			break;
		    }
		}
	    }

	    a = nt = 0;
	    for(nx=-1; nx<2; nx++)
		for(ny=-1; ny<2; ny++)
		    if(x+nx>=0 && y+ny>0 &&
		       x+nx<XRES && y+ny<YRES &&
		       (!bmap[(y+ny)/CELL][(x+nx)/CELL] || bmap[(y+ny)/CELL][(x+nx)/CELL]==5)) {
		        if(!pmap[y+ny][x+nx])
			    a = 1;
		        if((pmap[y+ny][x+nx]&0xFF)!=t)
			    nt = 1;
		    }

	    if(t==PT_GASS && pv[y/CELL][x/CELL]>4.0f)
		t = parts[i].type = PT_OILL;
	    if(t==PT_OILL && pv[y/CELL][x/CELL]<-4.0f)
		t = parts[i].type = PT_GASS;
	    if(t==PT_ICEI && pv[y/CELL][x/CELL]>0.8f)
		t = parts[i].type = PT_SNOW;
	    if(t==PT_PLUT && 1>rand()%100 && ((int)(5.0f*pv[y/CELL][x/CELL]))>(rand()%1000)) {
		t = PT_NEUT;
		create_part(i, x, y, t);
	    }

	    if(t==PT_WATR || t==PT_WIRE || t==PT_SPRK) {
		nx = x % CELL;
		if(nx == 0)
		    nx = x/CELL - 1;
		else if(nx == CELL-1)
		    nx = x/CELL + 1;
		else
		    nx = x/CELL;
		ny = y % CELL;
		if(ny == 0)
		    ny = y/CELL - 1;
		else if(ny == CELL-1)
		    ny = y/CELL + 1;
		else
		    ny = y/CELL;
		if(nx>=0 && ny>=0 && nx<XRES/CELL && ny<YRES/CELL) {
		    if(t==PT_WATR || t==PT_WIRE) {
			if(emap[ny][nx]==12 && !parts[i].life) {
			    parts[i].type = PT_SPRK;
			    parts[i].life = 4;
			    parts[i].ctype = t;
			    t = PT_SPRK;
			}
		    } else if(bmap[ny][nx]==6 || bmap[ny][nx]==7 || bmap[ny][nx]==3 || bmap[ny][nx]==8)
			set_emap(nx, ny);
		}
	    }

	    nx = x/CELL;
	    ny = y/CELL;
	    if(bmap[ny][nx]==6 && emap[ny][nx]<8)
		set_emap(nx, ny);

	    if(t==PT_ICEI || t==PT_SNOW) {
		for(nx=-2; nx<3; nx++)
		    for(ny=-2; ny<3; ny++)
			if(x+nx>=0 && y+ny>0 &&
			   x+nx<XRES && y+ny<YRES && (nx || ny)) {
			    r = pmap[y+ny][x+nx];
			    if((r>>8)>=NPART || !r)
				continue;
			    if((r&0xFF)==PT_WATR && 1>(rand()%1000)) {
				t = parts[i].type = PT_ICEI;
				parts[r>>8].type = PT_ICEI;
			    }
			    if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
				t = parts[i].type = PT_WATR;
			}
	    }

	    fe = 0;
	    if(t==PT_NEUT) {
		rt = 3 + (int)pv[y/CELL][x/CELL];
		for(nx=-1; nx<2; nx++)
		    for(ny=-1; ny<2; ny++)
			if(x+nx>=0 && y+ny>0 &&
			   x+nx<XRES && y+ny<YRES && (nx || ny)) {
			    r = pmap[y+ny][x+nx];
			    if((r>>8)>=NPART || !r)
				continue;
			    if((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) {
				parts[i].vx *= 0.995;
				parts[i].vy *= 0.995;
			    }
			    if((r&0xFF)==PT_PLUT && rt>(rand()%1000)) {
				if(33>rand()%100) {
				    create_part(r>>8, x+nx, y+ny, rand()%2 ? PT_LAVA : PT_FIRE);
				} else {
				    create_part(r>>8, x+nx, y+ny, PT_NEUT);
				    parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
				    parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
				}
				pv[y/CELL][x/CELL] += 2.00f * CFDS;
				fe ++;
			    }
			    if((r&0xFF)==PT_GUNP && 15>(rand()%1000))
				parts[r>>8].type = PT_DUST;
			    if((r&0xFF)==PT_PLEX && 15>(rand()%1000))
				parts[r>>8].type = PT_DFRM;
			    if((r&0xFF)==PT_NITR && 15>(rand()%1000))
				parts[r>>8].type = PT_OILL;
			    if((r&0xFF)==PT_OILL && 5>(rand()%1000))
				parts[r>>8].type = PT_GASS;
			}
	    }

	    if(t==PT_FIRE || t==PT_LAVA || t==PT_SPRK || fe) {
		for(nx=-2; nx<3; nx++)
		    for(ny=-2; ny<3; ny++)
			if(x+nx>=0 && y+ny>0 &&
			   x+nx<XRES && y+ny<YRES && (nx || ny)) {
			    r = pmap[y+ny][x+nx];
			    if((r>>8)>=NPART || !r)
				continue;
			    if(bmap[(y+ny)/CELL][(x+nx)/CELL] && bmap[(y+ny)/CELL][(x+nx)/CELL]!=5)
				continue;
			    rt = parts[r>>8].type;
			    if((a || explosive[rt]) &&
			       (t!=PT_LAVA || parts[i].life>0 || (rt!=PT_METL && rt!=PT_WIRE)) &&
			       flammable[rt] && (flammable[rt] + (int)(pv[(y+ny)/CELL][(x+nx)/CELL]*10.0f))>(rand()%1000)) {
				parts[r>>8].type = PT_FIRE;
				parts[r>>8].life = rand()%80+180;
				if(explosive[rt])
				    pv[y/CELL][x/CELL] += 0.25f * CFDS;
				continue;
			    }
			    lpv = (int)pv[(y+ny)/CELL][(x+nx)/CELL];
			    if(lpv < 1) lpv = 1;
			    if(t!=PT_SPRK && meltable[rt] && (t!=PT_FIRE || rt!=PT_WIRE) &&
			       meltable[rt]*lpv>(rand()%1000)) {
			        if(t!=PT_LAVA || parts[i].life>0) {
				    parts[r>>8].type = PT_LAVA;
				    parts[r>>8].life = rand()%120+240;
				} else {
				    parts[i].life = 0;
				    t = parts[i].type = rt;
				    goto killed;
				}
			    }
			    if(t!=PT_SPRK && (rt==PT_ICEI || rt==PT_SNOW)) {
				parts[r>>8].type = PT_WATR;
				if(t==PT_FIRE) {
				    parts[i].x = lx;
				    parts[i].y = ly;
				    kill_part(i);
				    goto killed;
				}
				if(t==PT_LAVA) {
				    parts[i].life = 0;
				    t = parts[i].type = PT_METL;
				    goto killed;
				}
			    }
			    if(t!=PT_SPRK && rt==PT_WATR) {
				kill_part(r>>8);
				if(t==PT_FIRE) {
				    parts[i].x = lx;
				    parts[i].y = ly;
				    kill_part(i);
				    goto killed;
				}
				if(t==PT_LAVA) {
				    parts[i].life = 0;
				    t = parts[i].type = PT_METL;
				    goto killed;
				}
			    }
			    if(t==PT_SPRK && rt==PT_WIRE && parts[r>>8].life==0 &&
			       (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4) {
				parts[r>>8].type = PT_SPRK;
				parts[r>>8].life = 4;
				parts[r>>8].ctype = rt;
			    }
			    if(t==PT_SPRK && rt==PT_WATR && parts[r>>8].life==0 &&
			       (parts[i].life<2 || ((r>>8)<i && parts[i].life<3)) && abs(nx)+abs(ny)<4) {
				parts[r>>8].type = PT_SPRK;
				parts[r>>8].life = 6;
				parts[r>>8].ctype = rt;
			    }
			}
	    killed:
		if(parts[i].type == PT_NONE)
		    continue;
	    }

	    if(t==PT_CLNE) {
		if(!parts[i].ctype) {
		    for(nx=-1; nx<2; nx++)
			for(ny=-1; ny<2; ny++)
			    if(x+nx>=0 && y+ny>0 &&
		               x+nx<XRES && y+ny<YRES &&
			       pmap[y+ny][x+nx] &&
			       (pmap[y+ny][x+nx]&0xFF)!=PT_CLNE &&
			       (pmap[y+ny][x+nx]&0xFF)!=0xFF)
				parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
		} else
		    create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
	    }

	    nx = (int)(parts[i].x+0.5f);
	    ny = (int)(parts[i].y+0.5f);

	    if(nx<CELL || nx>=XRES-CELL ||
	       ny<CELL || ny>=YRES-CELL) {
		parts[i].x = lx;
		parts[i].y = ly;
	        kill_part(i);
		continue;
	    }

	    rt = parts[i].flags & FLAG_STAGNANT;
	    parts[i].flags &= ~FLAG_STAGNANT;
	    if(!try_move(i, x, y, nx, ny)) {
		parts[i].x = lx;
		parts[i].y = ly;
		if(falldown[t]) {
		    if(nx!=x && try_move(i, x, y, nx, y)) {
			parts[i].x = ix;
			parts[i].vx *= collision[t];
			parts[i].vy *= collision[t];
		    } else if(ny!=y && try_move(i, x, y, x, ny)) {
			parts[i].y = iy;
			parts[i].vx *= collision[t];
			parts[i].vy *= collision[t];
		    } else {
			r = (rand()%2)*2-1;
			if(ny!=y && try_move(i, x, y, x+r, ny)) {
			    parts[i].x += r;
			    parts[i].y = iy;
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			} else if(ny!=y && try_move(i, x, y, x-r, ny)) {
			    parts[i].x -= r;
			    parts[i].y = iy;
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			} else if(nx!=x && try_move(i, x, y, nx, y+r)) {
			    parts[i].x = ix;
			    parts[i].y += r;
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			} else if(nx!=x && try_move(i, x, y, nx, y-r)) {
			    parts[i].x = ix;
			    parts[i].y -= r;
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			} else if(falldown[t]>1 && parts[i].vy>fabs(parts[i].vx)) {
			    s = 0;
			    if(!rt || nt)
				rt = 50;
			    else
				rt = 10;
			    for(j=x+r; j>=0 && j>=x-rt && j<x+rt && j<XRES; j+=r) {
				if(try_move(i, x, y, j, ny)) {
				    parts[i].x += j-x;
				    parts[i].y += ny-y;
				    x = j;
				    y = ny;
				    s = 1;
				    break;
				}
				if(try_move(i, x, y, j, y)) {
				    parts[i].x += j-x;
				    x = j;
				    s = 1;
				    break;
				}
				if((pmap[y][j]&255)!=t || (bmap[y/CELL][j/CELL] && bmap[y/CELL][j/CELL]!=5))
				    break;
			    }
			    if(parts[i].vy>0)
				r = 1;
			    else
				r = -1;
			    if(s)
				for(j=y+r; j>=0 && j<YRES && j>=y-rt && j<x+rt; j+=r) {
				    if(try_move(i, x, y, x, j)) {
					parts[i].y += j-y;
					break;
				    }
				    if((pmap[j][x]&255)!=t || (bmap[j/CELL][x/CELL] && bmap[j/CELL][x/CELL]!=5)) {
					s = 0;
					break;
				    }
				}
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			    if(!s)
				parts[i].flags |= FLAG_STAGNANT;
			} else {
			    parts[i].flags |= FLAG_STAGNANT;
			    parts[i].vx *= collision[t];
			    parts[i].vy *= collision[t];
			}
		    }
		} else {
		    parts[i].flags |= FLAG_STAGNANT;
		    if(nx>x+ISTP) nx=x+ISTP;
		    if(nx<x-ISTP) nx=x-ISTP;
		    if(ny>y+ISTP) ny=y+ISTP;
		    if(ny<y-ISTP) ny=y-ISTP;
		    if(t==PT_NEUT && 100>(rand()%1000)) {
			kill_part(i);
			continue;
		    } else if(try_move(i, x, y, 2*x-nx, ny)) {
			parts[i].x = 2*x-nx;
			parts[i].y = iy;
			parts[i].vx *= collision[t];
		    } else if(try_move(i, x, y, nx, 2*y-ny)) {
			parts[i].x = ix;
			parts[i].y = 2*y-ny;
			parts[i].vy *= collision[t];
		    } else {
			parts[i].vx *= collision[t];
			parts[i].vy *= collision[t];
		    }
		}
	    }

	justdraw:

	    nx = (int)(parts[i].x+0.5f);
	    ny = (int)(parts[i].y+0.5f);

	    if(nx<CELL || nx>=XRES-CELL ||
	       ny<CELL || ny>=YRES-CELL) {
	        kill_part(i);
		continue;
	    }

	    if(t==PT_NEUT) {
		if(cmode == 3) {
		    vid[ny*XRES+nx] = pcolors[t];
		    cg = 8;
		    cb = 12;
		    x = nx/CELL;
		    y = ny/CELL;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		} else {
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
	    } else if(t==PT_FIRE && parts[i].life) {
		if(cmode == 3) {
		    cr = parts[i].life / 4;
		    cg = parts[i].life / 16;
		    cb = parts[i].life / 32;
		    if(cr>255) cr = 255;
		    if(cg>192) cg = 212;
		    if(cb>128) cb = 192;
		    x = nx/CELL;
		    y = ny/CELL;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		} else {
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
	    } else if(t==PT_LAVA && parts[i].life) {
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
		if(cmode == 3) {
		    cr /= 32;
		    cg /= 32;
		    cb /= 32;
		    x = nx/CELL;
		    y = ny/CELL;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		}
	    } else if(t==PT_LAVA || t==PT_SPRK) {
		vid[ny*XRES+nx] = pcolors[t];
		if(cmode == 3) {
		    if(t == PT_LAVA) {
			cr = 3;
			cg = i%2;
			cb = 0;
		    } else {
			cr = 8;
			cg = 12;
			cb = 16;
		    }
		    x = nx/CELL;
		    y = ny/CELL;
		    cr += fire_r[y][x]; if(cr > 255) cr = 255; fire_r[y][x] = cr;
		    cg += fire_g[y][x]; if(cg > 255) cg = 255; fire_g[y][x] = cg;
		    cb += fire_b[y][x]; if(cb > 255) cb = 255; fire_b[y][x] = cb;
		}
	    } else
		vid[ny*XRES+nx] = pcolors[t];
	}

    for(y=0; y<YRES/CELL; y++)
	for(x=0; x<XRES/CELL; x++)
	    if(bmap[y][x]==5) {
		lx = x*CELL + CELL*0.5f;
		ly = y*CELL + CELL*0.5f;
		for(t=0; t<1024; t++) {
		    nx = (int)(lx+0.5f);
		    ny = (int)(ly+0.5f);
		    if(nx<0 || nx>=XRES || ny<0 || ny>=YRES)
			break;
		    addpixel(vid, nx, ny, 255, 255, 255, 64);
		    i = nx/CELL;
		    j = ny/CELL;
		    lx += vx[j][i]*0.125f;
		    ly += vy[j][i]*0.125f;
		    if(bmap[j][i]==5 && i!=x && j!=y)
			break;
		}
		drawtext(vid, x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
	    }
}

/***********************************************************
 *                       SDL OUTPUT                        *
 ***********************************************************/

int sdl_scale = 1;
SDL_Surface *sdl_scrn;
SDLMod sdl_mod;
int sdl_key, sdl_wheel, sdl_caps=0, sdl_ascii, sdl_zoom_trig=0;

#include "icon.h"
void sdl_seticon(void)
{
    SDL_Surface *icon =
	SDL_CreateRGBSurfaceFrom(app_icon, 32, 32, 24, 96,
				 0x0000FF, 0x00FF00, 0xFF0000, 0);
    SDL_WM_SetIcon(icon, NULL);
}

void sdl_open(void)
{
    if(SDL_Init(SDL_INIT_VIDEO)<0) {
        fprintf(stderr, "Initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
#ifdef PIX16
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale,YRES*sdl_scale + 60*sdl_scale,16,SDL_SWSURFACE);
#else
    sdl_scrn=SDL_SetVideoMode(XRES*sdl_scale,YRES*sdl_scale + 60*sdl_scale,32,SDL_SWSURFACE);
#endif
    if(!sdl_scrn) {
        fprintf(stderr, "Creating window: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_WM_SetCaption("The Powder Toy", "Powder Toy");
    sdl_seticon();
    SDL_EnableUNICODE(1);
}

void sdl_blit_1(int x, int y, int w, int h, pixel *src, int pitch)
{
    pixel *dst;
    unsigned j;
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
    for(j=0;j<h;j++) {
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
    unsigned j;
    unsigned i,k;
    if(SDL_MUSTLOCK(sdl_scrn))
        if(SDL_LockSurface(sdl_scrn)<0)
            return;
    dst=(pixel *)sdl_scrn->pixels+y*sdl_scrn->pitch/PIXELSIZE+x;
    for(j=0;j<h;j++) {
	for(k=0;k<sdl_scale;k++) {
	    for(i=0;i<w;i++) {
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

int frame_idx=0;
void dump_frame(pixel *src, int w, int h, int pitch)
{
    char frame_name[32];
    unsigned j,i;
    unsigned char c[3];
    FILE *f;
    sprintf(frame_name,"frame%04d.ppm",frame_idx);
    f=fopen(frame_name,"wb");
    fprintf(f,"P6\n%d %d\n255\n",w,h);
    for(j=0;j<h;j++) {
	for(i=0;i<w;i++) {
	    c[0] = PIXR(src[i]);
	    c[1] = PIXG(src[i]);
	    c[2] = PIXB(src[i]);
	    fwrite(c,3,1,f);
	}
	src+=pitch;
    }
    fclose(f);
    frame_idx++;
}

int Z_keysym = 'z';
int sdl_poll(void)
{
    SDL_Event event;
    sdl_key=sdl_wheel=sdl_ascii=0;
    while(SDL_PollEvent(&event)) {
        switch (event.type) {
	case SDL_KEYDOWN:
	    sdl_key=event.key.keysym.sym;
	    sdl_ascii=event.key.keysym.unicode;
	    if(event.key.keysym.sym == SDLK_CAPSLOCK)
		sdl_caps = 1;
	    if(event.key.keysym.unicode=='z' || event.key.keysym.unicode=='Z') {
		sdl_zoom_trig = 1;
		Z_keysym = event.key.keysym.sym;
	    }
	    break;
	case SDL_KEYUP:
	    if(event.key.keysym.sym == SDLK_CAPSLOCK)
		sdl_caps = 0;
	    if(event.key.keysym.sym == Z_keysym)
		sdl_zoom_trig = 0;
	    break;
	case SDL_MOUSEBUTTONDOWN:
	    if(event.button.button == SDL_BUTTON_WHEELUP)
		sdl_wheel++;
	    if(event.button.button == SDL_BUTTON_WHEELDOWN)
		sdl_wheel--;
	    break;
        case SDL_QUIT:
	    return 1;
        }
    }
    sdl_mod = SDL_GetModState();
    return 0;
}

/***********************************************************
 *                    STATE MANAGEMENT                     *
 ***********************************************************/

int svf_login = 0;
int svf_admin = 0;
char svf_user[64] = "";
char svf_pass[64] = "";

int svf_open = 0;
int svf_own = 0;
char svf_id[16] = "";
char svf_name[64] = "";
char svf_tags[256] = "";
void *svf_last = NULL;
int svf_lsize;

void *build_thumb(int *size, int bzip2)
{
    unsigned char *d=calloc(1,XRES*YRES), *c;
    int i,j,x,y;
    for(i=0; i<NPART; i++)
	if(parts[i].type) {
	    x = (int)(parts[i].x+0.5f);
	    y = (int)(parts[i].y+0.5f);
	    if(x>=0 && x<XRES && y>=0 && y<YRES)
		d[x+y*XRES] = parts[i].type;
	}
    for(y=0;y<YRES/CELL;y++)
	for(x=0;x<XRES/CELL;x++)
	    if(bmap[y][x])
		for(j=0;j<CELL;j++)
		    for(i=0;i<CELL;i++)
			d[x*CELL+i+(y*CELL+j)*XRES] = 0xFF;
    j = XRES*YRES;

    if(bzip2) {
	i = (j*101+99)/100 + 608;
	c = malloc(i);

	c[0] = 0x53;
	c[1] = 0x68;
	c[2] = 0x49;
	c[3] = 0x74;
	c[4] = PT_NUM;
	c[5] = CELL;
	c[6] = XRES/CELL;
	c[7] = YRES/CELL;

	i -= 8;

	if(BZ2_bzBuffToBuffCompress((char *)(c+8), (unsigned *)&i, (char *)d, j, 9, 0, 0) != BZ_OK) {
	    free(d);
	    free(c);
	    return NULL;
	}
	free(d);
	*size = i+8;
	return c;
    }

    *size = j;
    return d;
}

void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a);
int render_thumb(void *thumb, int size, int bzip2, pixel *vid_buf, int px, int py, int scl)
{
    unsigned char *d,*c=thumb;
    int i,j,x,y,a,t,r,g,b,sx,sy;

    if(bzip2) {
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
    } else
	d = c;

    if(size < XRES*YRES) {
	if(bzip2)
	    free(d);
	return 1;
    }

    sy = 0;
    for(y=0;y+scl<=YRES;y+=scl) {
	sx = 0;
	for(x=0;x+scl<=XRES;x+=scl) {
	    a = 0;
	    r = g = b = 0;
	    for(j=0;j<scl;j++)
		for(i=0;i<scl;i++) {
		    t = d[(y+j)*XRES+(x+i)];
		    if(t==0xFF) {
			r += 256;
			g += 256;
			b += 256;
			a += 2;
		    } else if(t) {
			if(t>=PT_NUM)
			    goto corrupt;
			r += PIXR(pcolors[t]);
			g += PIXG(pcolors[t]);
			b += PIXB(pcolors[t]);
			a ++;
		    }
		}
	    if(a) {
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

static char *mystrdup(char *s)
{
    char *x;
    if(s) {
	x = malloc(strlen(s)+1);
	strcpy(x, s);
	return x;
    }
    return s;
}

void *build_save(int *size, int x0, int y0, int w, int h)
{
    unsigned char *d=calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*6+MAXSIGNS*262), *c;
    int i,j,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int));
    int bx0=x0/CELL, by0=y0/CELL, bw=(w+CELL-1)/CELL, bh=(h+CELL-1)/CELL;

    // normalize coordinates
    x0 = bx0*CELL;
    y0 = by0*CELL;
    w  = bw *CELL;
    h  = bh *CELL;

    // save the required air state
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++)
	    d[p++] = bmap[y][x];
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++)
	    if(bmap[y][x]==4) {
		i = (int)(fvx[y][x]*64.0+127.5);
		if(i<0) i=0;
		if(i>255) i=255;
		d[p++] = i;
	    }
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++)
	    if(bmap[y][x]==4) {
		i = (int)(fvy[y][x]*64.0+127.5);
		if(i<0) i=0;
		if(i>255) i=255;
		d[p++] = i;
	    }

    // save the particle map
    for(i=0; i<NPART; i++)
	if(parts[i].type) {
	    x = (int)(parts[i].x+0.5f);
	    y = (int)(parts[i].y+0.5f);
	    if(x>=x0 && x<x0+w && y>=y0 && y<y0+h)
		m[(x-x0)+(y-y0)*w] = i+1;
	}
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i)
	    d[p++] = parts[i-1].type;
	else
	    d[p++] = 0;
    }

    // save particle properties
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i) {
	    i--;
	    x = (int)(parts[i].vx*16.0+127.5);
	    y = (int)(parts[i].vy*16.0+127.5);
	    if(x<0) x=0;
	    if(x>255) x=255;
	    if(y<0) y=0;
	    if(y>255) y=255;
	    d[p++] = x;
	    d[p++] = y;
	}
    }
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i)
	    d[p++] = (parts[i-1].life+3)/4;
    }
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i && (parts[i-1].type==PT_CLNE || parts[i-1].type==PT_SPRK))
	    d[p++] = parts[i-1].ctype;
    }

    j = 0;
    for(i=0; i<MAXSIGNS; i++)
	if(signs[i].text[0] &&
	   signs[i].x>=x0 && signs[i].x<x0+w &&
	   signs[i].y>=y0 && signs[i].y<y0+h)
	    j++;
    d[p++] = j;
    for(i=0; i<MAXSIGNS; i++)
	if(signs[i].text[0] &&
	   signs[i].x>=x0 && signs[i].x<x0+w &&
	   signs[i].y>=y0 && signs[i].y<y0+h) {
	    d[p++] = (signs[i].x-x0);
	    d[p++] = (signs[i].x-x0)>>8;
	    d[p++] = (signs[i].y-y0);
	    d[p++] = (signs[i].y-y0)>>8;
	    d[p++] = signs[i].ju;
	    x = strlen(signs[i].text);
	    d[p++] = x;
	    memcpy(d+p, signs[i].text, x);
	    p+=x;
	}

    i = (p*101+99)/100 + 612;
    c = malloc(i);
    c[0] = 0x66;
    c[1] = 0x75;
    c[2] = 0x43;
    c[3] = 0x4B;
    c[4] = SAVE_VERSION;
    c[5] = CELL;
    c[6] = bw;
    c[7] = bh;
    c[8] = p;
    c[9] = p >> 8;
    c[10] = p >> 16;
    c[11] = p >> 24;

    i -= 12;

    if(BZ2_bzBuffToBuffCompress((char *)(c+12), (unsigned *)&i, (char *)d, p, 9, 0, 0) != BZ_OK) {
	free(d);
	free(c);
	return NULL;
    }
    free(d);

    *size = i+12;
    return c;
}

int parse_save(void *save, int size, int replace, int x0, int y0)
{
    unsigned char *d,*c=save;
    int i,j,k,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int)), ver, pty, ty;
    int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
    int fp[NPART], nf=0;

    if(size<16)
	return 1;
    if(c[3]!=0x4B || c[2]!=0x43 || c[1]!=0x75 || c[0]!=0x66)
	return 1;
    if(c[4]>SAVE_VERSION)
	return 2;
    ver = c[4];

    bw = c[6];
    bh = c[7];
    if(bx0+bw > XRES/CELL)
	bx0 = XRES/CELL - bw;
    if(by0+bh > YRES/CELL)
	by0 = YRES/CELL - bh;
    if(bx0 < 0)
	bx0 = 0;
    if(by0 < 0)
	by0 = 0;

    if(c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
	return 3;
    i = (unsigned)c[8];
    i |= ((unsigned)c[9])<<8;
    i |= ((unsigned)c[10])<<16;
    i |= ((unsigned)c[11])<<24;
    d = malloc(i);
    if(!d)
	return 1;

    if(BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
	return 1;
    size = i;

    if(size < bw*bh)
	return 1;

    // normalize coordinates
    x0 = bx0*CELL;
    y0 = by0*CELL;
    w  = bw *CELL;
    h  = bh *CELL;

    if(replace) {
	memset(bmap, 0, sizeof(bmap));
	memset(emap, 0, sizeof(emap));
	memset(signs, 0, sizeof(signs));
	memset(parts, 0, sizeof(particle)*NPART);
	memset(pmap, 0, sizeof(pmap));
	memset(vx, 0, sizeof(vx));
	memset(vy, 0, sizeof(vy));
	memset(pv, 0, sizeof(pv));
    }

    // make a catalog of free parts
    memset(pmap, 0, sizeof(pmap));
    for(i=0; i<NPART; i++)
	if(parts[i].type) {
	    x = (int)(parts[i].x+0.5f);
	    y = (int)(parts[i].y+0.5f);
	    pmap[y][x] = (i<<8)|1;
	} else
	    fp[nf++] = i;

    // load the required air state
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++) {
	    if(d[p])
		bmap[y][x] = d[p];
	    p++;
	}
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++)
	    if(d[(y-by0)*bw+(x-bx0)]==4) {
		if(p >= size)
		    goto corrupt;
		fvx[y][x] = (d[p++]-127.0)/64.0;
	    }
    for(y=by0; y<by0+bh; y++)
	for(x=bx0; x<bx0+bw; x++)
	    if(d[(y-by0)*bw+(x-bx0)]==4) {
		if(p >= size)
		    goto corrupt;
		fvy[y][x] = (d[p++]-127.0)/64.0;
	    }

    // load the particle map
    i = 0;
    pty = p;
    for(y=y0;y<y0+h;y++)
	for(x=x0;x<x0+w;x++) {
	    if(p >= size)
		goto corrupt;
	    j=d[p++];
	    if(j >= PT_NUM)
		goto corrupt;
	    if(j) {
		if(pmap[y][x]) {
		    k = pmap[y][x]>>8;
		    parts[k].type = j;
		    parts[k].x = x;
		    parts[k].y = y;
		    m[(x-x0)+(y-y0)*w] = k+1;
		} else if(i < nf) {
		    parts[fp[i]].type = j;
		    parts[fp[i]].x = x;
		    parts[fp[i]].y = y;
		    m[(x-x0)+(y-y0)*w] = fp[i]+1;
		    i++;
		} else
		    m[(x-x0)+(y-y0)*w] = NPART+1;
	    }
	}

    // load particle properties
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i) {
	    i--;
	    if(p+1 >= size)
		goto corrupt;
	    if(i < NPART) {
		parts[i].vx = (d[p++]-127.0)/16.0;
		parts[i].vy = (d[p++]-127.0)/16.0;
	    } else
		p += 2;
	}
    }
    for(j=0; j<w*h; j++) {
	i = m[j];
	if(i) {
	    if(p >= size)
		goto corrupt;
	    if(i <= NPART)
		parts[i-1].life = d[p++]*4;
	    else
		p++;
	}
    }
    for(j=0; j<w*h; j++) {
	i = m[j];
	ty = d[pty+j];
	if(i && (ty==PT_CLNE || (ty==PT_SPRK && ver>=21))) {
	    if(p >= size)
		goto corrupt;
	    if(i <= NPART)
		parts[i-1].ctype = d[p++];
	    else
		p++;
	}
    }

    if(p >= size)
	goto version1;
    j = d[p++];
    for(i=0; i<j; i++) {
	if(p+6 > size)
	    goto corrupt;
	for(k=0; k<MAXSIGNS; k++)
	    if(!signs[k].text[0])
		break;
	x = d[p++];
	x |= ((unsigned)d[p++])<<8;
	if(k<MAXSIGNS)
	    signs[k].x = x+x0;
	x = d[p++];
	x |= ((unsigned)d[p++])<<8;
	if(k<MAXSIGNS)
	    signs[k].y = x+y0;
	x = d[p++];
	if(k<MAXSIGNS)
	    signs[k].ju = x;
	x = d[p++];
	if(p+x > size)
	    goto corrupt;
	if(k<MAXSIGNS) {
	    memcpy(signs[k].text, d+p, x);
	    signs[k].text[x] = 0;
	}
	p += x;
    }

version1:
    free(d);

    return 0;

corrupt:
    if(replace) {
	memset(signs, 0, sizeof(signs));
	memset(parts, 0, sizeof(particle)*NPART);
	memset(bmap, 0, sizeof(bmap));
    }
    return 1;
}

pixel *prerender_save(void *save, int size, int *width, int *height)
{
    unsigned char *d,*c=save;
    int i,j,k,x,y,rx,ry,p=0;
    int bw,bh,w,h;
    pixel *fb;

    if(size<16)
	return NULL;
    if(c[3]!=0x4B || c[2]!=0x43 || c[1]!=0x75 || c[0]!=0x66)
	return NULL;
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
    if(!fb) {
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
	for(x=0; x<bw; x++) {
	    rx = x*CELL;
	    ry = y*CELL;
	    switch(d[p]) {
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

    for(y=0;y<h;y++)
	for(x=0;x<w;x++) {
	    if(p >= size)
		goto corrupt;
	    j=d[p++];
	    if(j<PT_NUM && j>0)
		fb[y*w+x] = pcolors[j];
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

// FBI. I'm not kidding.

#include "fbi.h"

pixel *render_packed_rgb(void *image, int width, int height, int cmp_size)
{
    unsigned char *tmp;
    pixel *res;
    int i;

    tmp = malloc(width*height*3);
    if(!tmp)
	return NULL;
    res = malloc(width*height*PIXELSIZE);
    if(!res) {
	free(tmp);
	return NULL;
    }

    i = width*height*3;
    if(BZ2_bzBuffToBuffDecompress((char *)tmp, (unsigned *)&i, (char *)image, cmp_size, 0, 0)) {
	free(res);
	free(tmp);
	return NULL;
    }

    for(i=0; i<width*height; i++)
	res[i] = PIXRGB(tmp[3*i], tmp[3*i+1], tmp[3*i+2]);

    free(tmp);
    return res;
}

// stamps library

#define STAMP_X 4
#define STAMP_Y 4
struct stamp_info {
    char name[11];
    pixel *thumb;
    int thumb_w, thumb_h;
} stamps[STAMP_X*STAMP_Y];

unsigned last_time=0, last_name=0;
void stamp_gen_name(char *fn)
{
    unsigned t=(unsigned)time(NULL);

    if(last_time!=t) {
	last_time=t;
	last_name=0;
    } else
	last_name++;

    sprintf(fn, "%08x%02x", last_time, last_name);
}

void *file_load(char *fn, int *size)
{
    FILE *f = fopen(fn, "rb");
    void *s;

    if(!f)
	return NULL;
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    s = malloc(*size);
    if(!s) {
	fclose(f);
	return NULL;
    }
    fread(s, *size, 1, f);
    fclose(f);
    return s;
}

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

void stamp_update(void)
{
    FILE *f;
    int i;
    f=fopen("stamps" PATH_SEP "stamps.def", "wb");
    if(!f)
	return;
    for(i=0; i<STAMP_X*STAMP_Y; i++) {
	if(!stamps[i].name[0])
	    break;
	fwrite(stamps[i].name, 1, 10, f);
    }
    fclose(f);
}

pixel *rescale_img(pixel *src, int sw, int sh, int *qw, int *qh, int f)
{
    int i,j,x,y,w,h,r,g,b,c;
    pixel p, *q;
    w = (sw+f-1)/f;
    h = (sh+f-1)/f;
    q = malloc(w*h*PIXELSIZE);
    for(y=0; y<h; y++)
	for(x=0; x<w; x++) {
	    r = g = b = c = 0;
	    for(j=0; j<f; j++)
		for(i=0; i<f; i++)
		    if(x*f+i<sw && y*f+j<sh) {
			p = src[(y*f+j)*sw + (x*f+i)];
			if(p) {
			    r += PIXR(p);
			    g += PIXG(p);
			    b += PIXB(p);
			    c ++;
			}
		    }
	    if(c>1) {
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

void stamp_gen_thumb(int i)
{
    char fn[64];
    void *data;
    int size, factor_x, factor_y;
    pixel *tmp;

    if(stamps[i].thumb) {
	free(stamps[i].thumb);
	stamps[i].thumb = NULL;
    }

    sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
    data = file_load(fn, &size);

    if(data) {
	stamps[i].thumb = prerender_save(data, size, &(stamps[i].thumb_w), &(stamps[i].thumb_h));
	if(stamps[i].thumb && (stamps[i].thumb_w>XRES/STAMP_X || stamps[i].thumb_h>YRES/STAMP_Y)) {
	    factor_x = (stamps[i].thumb_w+XRES/STAMP_X-1) / (XRES/STAMP_X);
	    factor_y = (stamps[i].thumb_h+YRES/STAMP_Y-1) / (YRES/STAMP_Y);
	    if(factor_y > factor_x)
		factor_x = factor_y;
	    tmp = rescale_img(stamps[i].thumb, stamps[i].thumb_w, stamps[i].thumb_h, &(stamps[i].thumb_w), &(stamps[i].thumb_h), factor_x);
	    free(stamps[i].thumb);
	    stamps[i].thumb = tmp;
	}
    }

    free(data);
}

void stamp_save(int x, int y, int w, int h)
{
    FILE *f;
    int n;
    char fn[64], sn[16];
    void *s=build_save(&n, x, y, w, h);

#ifdef WIN32
    _mkdir("stamps");
#else
    mkdir("stamps", 0755);
#endif

    stamp_gen_name(sn);
    sprintf(fn, "stamps" PATH_SEP "%s.stm", sn);

    f = fopen(fn, "wb");
    if(!f)
	return;
    fwrite(s, n, 1, f);
    fclose(f);

    free(s);

    if(stamps[STAMP_X*STAMP_Y-1].thumb)
	free(stamps[STAMP_X*STAMP_Y-1].thumb);
    memmove(stamps+1, stamps, sizeof(struct stamp_info)*(STAMP_X*STAMP_Y-1));
    memset(stamps, 0, sizeof(struct stamp_info));

    strcpy(stamps[0].name, sn);
    stamp_gen_thumb(0);

    stamp_update();
}

void *stamp_load(int i, int *size)
{
    void *data;
    char fn[64];
    struct stamp_info tmp;

    if(!stamps[i].thumb || !stamps[i].name[0])
	return NULL;

    sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
    data = file_load(fn, size);
    if(!data)
	return NULL;

    if(i>0) {
	memcpy(&tmp, stamps+i, sizeof(struct stamp_info));
	memmove(stamps+1, stamps, sizeof(struct stamp_info)*i);
	memcpy(stamps, &tmp, sizeof(struct stamp_info));

	stamp_update();
    }

    return data;
}

void stamp_init(void)
{
    int i;
    FILE *f;

    memset(stamps, 0, sizeof(stamps));

    f=fopen("stamps" PATH_SEP "stamps.def", "rb");
    if(!f)
	return;
    for(i=0; i<STAMP_X*STAMP_Y; i++) {
	fread(stamps[i].name, 1, 10, f);
	if(!stamps[i].name[0])
	    break;
	stamp_gen_thumb(i);
    }
    fclose(f);
}

/***********************************************************
 *                      FONT DRAWING                       *
 ***********************************************************/

#include "font.h"

void drawpixel(pixel *vid, int x, int y, int r, int g, int b, int a)
{
    pixel t;
    if(x<0 || y<0 || x>=XRES || y>=YRES+60)
	return;
    if(a!=255) {
	t = vid[y*XRES+x];
	r = (a*r + (255-a)*PIXR(t)) >> 8;
	g = (a*g + (255-a)*PIXG(t)) >> 8;
	b = (a*b + (255-a)*PIXB(t)) >> 8;
    }
    vid[y*XRES+x] = PIXRGB(r,g,b);
}

int drawchar(pixel *vid, int x, int y, int c, int r, int g, int b, int a)
{
    int i, j, w, bn = 0, ba = 0;
    char *rp = font_data + font_ptrs[c];
    w = *(rp++);
    for(j=0; j<FONT_H; j++)
	for(i=0; i<w; i++) {
	    if(!bn) {
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
    for(;*s;s++) {
	if(*s == '\n') {
	    x = sx;
	    y += FONT_H+2;
	} else if(*s == '\b') {
	    switch(s[1]) {
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
	} else
	    x = drawchar(vid, x, y, *(unsigned char *)s, r, g, b, a);
    }
    return x;
}

void drawrect(pixel *vid, int x, int y, int w, int h, int r, int g, int b, int a)
{
    int i;
    for(i=0; i<=w; i++) {
	drawpixel(vid, x+i, y, r, g, b, a);
	drawpixel(vid, x+i, y+h, r, g, b, a);
    }
    for(i=1; i<h; i++) {
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
	memset(vid+(x+1+XRES*(y+i)), 0, PIXELSIZE*(w-1));
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
    for(;*s;s++)
	x += font_data[font_ptrs[(int)(*(unsigned char *)s)]];
    return x-1;
}
int drawtextmax(pixel *vid, int x, int y, int w, char *s, int r, int g, int b, int a)
{
    int i;
    w += x-5;
    for(;*s;s++) {
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
    for(;*s;s++) {
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
    for(;*s;s++) {
	cw = font_data[font_ptrs[(int)(*(unsigned char *)s)]];
	if(x+(cw/2) >= w)
	    break;
	x += cw;
	n++;
    }
    return n;
}

/***********************************************************
 *                      MAIN PROGRAM                       *
 ***********************************************************/

void draw_tool(pixel *vid_buf, int b, int sl, int sr, unsigned pc)
{
    int x, y, i, j, c;
    x = 2+32*(b/2);
    y = YRES+2+20*(b%2);

    if(b>=PT_NUM)
	x -= 512-XRES;

    if(b==22)
	for(j=1; j<15; j++)
	    for(i=1; i<27; i++) {
		if(!(i%2) && !(j%2))
		    vid_buf[XRES*(y+j)+(x+i)] = pc;
		else
		    vid_buf[XRES*(y+j)+(x+i)] = 0x808080;
	    }
    else if(b==23) {
	for(j=1; j<15; j++) {
	    for(i=1; i<6+j; i++)
		if(!(i&j&1))
		    vid_buf[XRES*(y+j)+(x+i)] = pc;
	    for(; i<27; i++)
		if(i&j&1)
		    vid_buf[XRES*(y+j)+(x+i)] = pc;
	}
    } else if(b==24)
	for(j=1; j<15; j+=2)
	    for(i=1+(1&(j>>1)); i<27; i+=2)
		vid_buf[XRES*(y+j)+(x+i)] = pc;
    else if(b==25) {
	for(j=1; j<15; j++)
	    for(i=1; i<27; i++)
		vid_buf[XRES*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? 0xA0A0A0 : 0x000000;
	drawtext(vid_buf, x+4, y+3, "\x8D", 255, 255, 255, 255);
	for(i=9; i<27; i++)
	    drawpixel(vid_buf, x+i, y+8+(int)(3.9f*cos(i*0.3f)), 255, 255, 255, 255);
    } else if(b==26) {
	for(j=1; j<15; j++)
	    for(i=1; i<27; i++)
		vid_buf[XRES*(y+j)+(x+i)] = i==1||i==26||j==1||j==14 ? 0xA0A0A0 : 0x000000;
	drawtext(vid_buf, x+9, y+3, "\xA1", 32, 64, 128, 255);
	drawtext(vid_buf, x+9, y+3, "\xA0", 255, 255, 255, 255);
    } else if(b==27)
	for(j=1; j<15; j+=2)
	    for(i=1+(1&(j>>1)); i<27; i+=2)
		vid_buf[XRES*(y+j)+(x+i)] = pc;
    else if(b==28)
	for(j=1; j<15; j++)
	    for(i=1; i<27; i++) {
		if(!(i%2) && !(j%2))
		    vid_buf[XRES*(y+j)+(x+i)] = pc;
	    }
    else if(b==29)
	for(j=1; j<15; j+=2)
	    for(i=1+(1&(j>>1)); i<27; i+=2)
		vid_buf[XRES*(y+j)+(x+i)] = pc;
    else if(b==30) {
	for(j=1; j<15; j+=2)
	    for(i=1+(1&(j>>1)); i<13; i+=2)
		vid_buf[XRES*(y+j)+(x+i)] = pc;
	for(j=1; j<15; j++)
	    for(i=14; i<27; i++)
		vid_buf[XRES*(y+j)+(x+i)] = pc;
    } else
	for(j=1; j<15; j++)
	    for(i=1; i<27; i++)
		vid_buf[XRES*(y+j)+(x+i)] = pc;

    if(b==30 || b==0)
	for(j=4; j<12; j++) {
	    vid_buf[XRES*(y+j)+(x+j+6)] = PIXPACK(0xFF0000);
	    vid_buf[XRES*(y+j)+(x+j+7)] = PIXPACK(0xFF0000);
	    vid_buf[XRES*(y+j)+(x-j+21)] = PIXPACK(0xFF0000);
	    vid_buf[XRES*(y+j)+(x-j+22)] = PIXPACK(0xFF0000);
	}

    if(b>0 && b<PT_NUM) {
	c = PIXB(pcolors[b]) + 3*PIXG(pcolors[b]) + 2*PIXR(pcolors[b]);
	if(c<512)
	    c = 255;
	else
	    c = 0;
	drawtext(vid_buf, x+14-textwidth(names[b])/2, y+4, names[b], c, c, c, 255);
    }

    if(b==sl || b==sr) {
	c = 0;
	if(b==sl)
	    c |= PIXPACK(0xFF0000);
	if(b==sr)
	    c |= PIXPACK(0x0000FF);
	for(i=0; i<30; i++) {
	    vid_buf[XRES*(y-1)+(x+i-1)] = c;
	    vid_buf[XRES*(y+16)+(x+i-1)] = c;
	}
	for(j=0; j<18; j++) {
	    vid_buf[XRES*(y+j-1)+(x-1)] = c;
	    vid_buf[XRES*(y+j-1)+(x+28)] = c;
	}
    }
}

int create_parts(int x, int y, int r, int c)
{
    int i, j, f = 0, u, v;

    if(c == 25) {
	i = x / CELL;
	j = y / CELL;
	for(v=-1;v<2;v++)
	    for(u=-1;u<2;u++)
		if(i+u>=0 && i+u<XRES/CELL &&
		   j+v>=0 && j+v<YRES/CELL &&
	           bmap[j+v][i+u] == 5)
		    return 1;
	bmap[j][i] = 5;
	return 1;
    }
    if(c == 27) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 4;
	fvx[j][i] = 0.0f;
	fvy[j][i] = 0.0f;
	return 1;
    }
    if(c == 22) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 8;
	return 1;
    }
    if(c == 23) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 7;
	return 1;
    }
    if(c == 24) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 6;
	return 1;
    }
    if(c == 28) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 3;
	return 1;
    }
    if(c == 29) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 2;
	return 1;
    }
    if(c == 30) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 0;
	return 1;
    }
    if(c == 31) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 1;
	return 1;
    }
    if(c == 255) {
	i = x / CELL;
	j = y / CELL;
	bmap[j][i] = 255;
	return 1;
    }

    if(c == 0) {
	for(j=-r; j<=r; j++)
	    for(i=-r; i<=r; i++)
		if(i*i+j*j<=r*r)
		    delete_part(x+i, y+j);
	return 1;
    }

    for(j=-r; j<=r; j++)
	for(i=-r; i<=r; i++)
	    if(i*i+j*j<=r*r)
		if(create_part(-1, x+i, y+j, c)==-1)
		    f = 1;
    return !f;
}

void create_line(int x1, int y1, int x2, int y2, int r, int c)
{
    int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
    float e, de;
    if(cp) {
	y = x1;
	x1 = y1;
	y1 = y;
	y = x2;
	x2 = y2;
	y2 = y;
    }
    if(x1 > x2) {
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
    for(x=x1; x<=x2; x++) {
	if(cp)
	    create_parts(y, x, r, c);
	else
	    create_parts(x, y, r, c);
	e += de;
	if(e >= 0.5f) {
	    y += sy;
	    if(c==31 || c==29 || c==28 || c==27 || c==25 || c==24 || c==23 || c==22 || !r) {
		if(cp)
		    create_parts(y, x, r, c);
		else
		    create_parts(x, y, r, c);
	    }
	    e -= 1.0f;
	}
    }
}

void create_box(int x1, int y1, int x2, int y2, int c)
{
    int i, j;
    if(x1>x2) {
	i = x2;
	x2 = x1;
	x1 = i;
    }
    if(y1>y2) {
	j = y2;
	y2 = y1;
	y1 = j;
    }
    for(j=y1; j<=y2; j++)
	for(i=x1; i<=x2; i++)
	    create_parts(i, j, 1, c);
}

int flood_parts(int x, int y, int c, int cm, int bm)
{
    int x1, x2, dy = (c<PT_NUM)?1:CELL;

    if(cm==-1) {
	if(c==0) {
	    cm = pmap[y][x]&0xFF;
	    if(!cm)
		return 0;
	} else
	    cm = 0;
    }
    if(bm==-1) {
	if(c==30) {
	    bm = bmap[y/CELL][x/CELL];
	    if(!bm)
		return 0;
	    if(bm==1)
		cm = 0xFF;
	} else
	    bm = 0;
    }

    if((pmap[y][x]&0xFF)!=cm || bmap[y/CELL][x/CELL]!=bm)
	return 1;

    // go left as far as possible
    x1 = x2 = x;
    while(x1>=CELL) {
	if((pmap[y][x1-1]&0xFF)!=cm || bmap[y/CELL][(x1-1)/CELL]!=bm)
	    break;
	x1--;
    }
    while(x2<XRES-CELL) {
	if((pmap[y][x2+1]&0xFF)!=cm || bmap[y/CELL][(x2+1)/CELL]!=bm)
	    break;
	x2++;
    }

    // fill span
    for(x=x1; x<=x2; x++)
	if(!create_parts(x, y, 0, c))
	    return 0;

    // fill children
    if(y>=CELL+dy)
	for(x=x1; x<=x2; x++)
	    if((pmap[y-dy][x]&0xFF)==cm && bmap[(y-dy)/CELL][x/CELL]==bm)
		if(!flood_parts(x, y-dy, c, cm, bm))
		    return 0;
    if(y<YRES-CELL-dy)
	for(x=x1; x<=x2; x++)
	    if((pmap[y+dy][x]&0xFF)==cm && bmap[(y+dy)/CELL][x/CELL]==bm)
		if(!flood_parts(x, y+dy, c, cm, bm))
		    return 0;
    return 1;
}

static void xor_pixel(int x, int y, pixel *vid)
{
    int c;
    if(x<0 || y<0 || x>=XRES || y>=YRES)
	return;
    c = vid[y*XRES+x];
    c = PIXB(c) + 3*PIXG(c) + 2*PIXR(c);
    if(c<512)
	vid[y*XRES+x] = PIXPACK(0xC0C0C0);
    else
	vid[y*XRES+x] = PIXPACK(0x404040);
}

void xor_line(int x1, int y1, int x2, int y2, pixel *vid)
{
    int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
    float e, de;
    if(cp) {
	y = x1;
	x1 = y1;
	y1 = y;
	y = x2;
	x2 = y2;
	y2 = y;
    }
    if(x1 > x2) {
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
    for(x=x1; x<=x2; x++) {
	if(cp)
	    xor_pixel(y, x, vid);
	else
	    xor_pixel(x, y, vid);
	e += de;
	if(e >= 0.5f) {
	    y += sy;
	    e -= 1.0f;
	}
    }
}

void draw_svf_ui(pixel *vid_buf)
{
    int c;

    drawtext(vid_buf, 4, YRES+44, "\x81", 255, 255, 255, 255);
    drawrect(vid_buf, 1, YRES+42, 16, 14, 255, 255, 255, 255);

    c = svf_open ? 255 : 128;
    drawtext(vid_buf, 23, YRES+44, "\x91", c, c, c, 255);
    drawrect(vid_buf, 19, YRES+42, 16, 14, c, c, c, 255);

    c = svf_login ? 255 : 128;
    drawtext(vid_buf, 40, YRES+44, "\x82", c, c, c, 255);
    if(svf_open)
	drawtext(vid_buf, 58, YRES+46, svf_name, c, c, c, 255);
    else
	drawtext(vid_buf, 58, YRES+46, "[untitled simulation]", c, c, c, 255);
    drawrect(vid_buf, 37, YRES+42, 150, 14, c, c, c, 255);
    if(svf_open && svf_own)
	drawdots(vid_buf, 55, YRES+43, 12, c, c, c, 255);

    c = (svf_login && svf_open) ? 255 : 128;
    drawtext(vid_buf, 192, YRES+44, "\x83", c, c, c, 255);
    if(svf_tags[0])
	drawtextmax(vid_buf, 210, YRES+46, 154, svf_tags, c, c, c, 255);
    else
	drawtext(vid_buf, 210, YRES+46, "[no tags set]", c, c, c, 255);
    drawrect(vid_buf, 189, YRES+42, 176, 14, c, c, c, 255);

    drawtext(vid_buf, 371, YRES+44, "\x92", 255, 255, 255, 255);
    drawrect(vid_buf, 367, YRES+42, 16, 14, 255, 255, 255, 255);

    drawtext(vid_buf, 388, YRES+45, "\x84", 255, 255, 255, 255);
    if(svf_login)
	drawtext(vid_buf, 406, YRES+46, svf_user, 255, 255, 255, 255);
    else
	drawtext(vid_buf, 406, YRES+46, "[sign in]", 255, 255, 255, 255);
    drawrect(vid_buf, 385, YRES+42, 91, 14, 255, 255, 255, 255);

    if(sys_pause) {
	fillrect(vid_buf, 493, YRES+41, 16, 16, 255, 255, 255, 255);
	drawtext(vid_buf, 496, YRES+44, "\x90", 0, 0, 0, 255);
    } else {
	drawtext(vid_buf, 496, YRES+44, "\x90", 255, 255, 255, 255);
	drawrect(vid_buf, 494, YRES+42, 14, 14, 255, 255, 255, 255);
    }

    switch(cmode) {
    case 0:
	drawtext(vid_buf, 481, YRES+45, "\x98", 128, 160, 255, 255);
	break;
    case 1:
	drawtext(vid_buf, 481, YRES+45, "\x99", 255, 212, 32, 255);
	break;
    case 2:
	drawtext(vid_buf, 481, YRES+45, "\x9A", 212, 212, 212, 255);
	break;
    case 3:
	drawtext(vid_buf, 481, YRES+45, "\x9B", 255, 0, 0, 255);
	drawtext(vid_buf, 481, YRES+45, "\x9C", 255, 255, 64, 255);
	break;
    }
    drawrect(vid_buf, 478, YRES+42, 14, 14, 255, 255, 255, 255);

    if(svf_admin) {
	drawtext(vid_buf, 463, YRES+44, "\xA4", 0, 32, 0, 255);
	drawtext(vid_buf, 463, YRES+44, "\xA3", 0, 255, 127, 255);
    } else if(amd)
	drawtext(vid_buf, 465, YRES+45, "\x97", 0, 230, 153, 255);
}

typedef struct ui_edit {
    int x, y, w, nx;
    char str[256],*def;
    int focus, cursor, hide;
} ui_edit;
void ui_edit_draw(pixel *vid_buf, ui_edit *ed)
{
    int cx, i;
    char echo[256], *str;

    if(ed->hide) {
	for(i=0;ed->str[i];i++)
	    echo[i] = 0x8D;
	echo[i] = 0;
	str = echo;
    } else
	str = ed->str;

    if(ed->str[0]) {
	drawtext(vid_buf, ed->x, ed->y, str, 255, 255, 255, 255);
	drawtext(vid_buf, ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
    } else if(!ed->focus)
	drawtext(vid_buf, ed->x, ed->y, ed->def, 128, 128, 128, 255);
    if(ed->focus) {
	cx = textnwidth(str, ed->cursor);
	for(i=-3; i<9; i++)
	    drawpixel(vid_buf, ed->x+cx, ed->y+i, 255, 255, 255, 255);
    }
}
char *shift_0="`1234567890-=[]\\;',./";
char *shift_1="~!@#$%^&*()_+{}|:\"<>?";
void ui_edit_process(int mx, int my, int mb, ui_edit *ed)
{
    char ch, ts[2], echo[256], *str;
    int l, i;
#ifdef RAWINPUT
    char *p;
#endif

    if(mb) {
	if(ed->hide) {
	    for(i=0;ed->str[i];i++)
		echo[i] = 0x8D;
	    echo[i] = 0;
	    str = echo;
	} else
	    str = ed->str;

	if(mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11) {
	    ed->focus = 1;
	    ed->cursor = 0;
	    ed->str[0] = 0;
	} else if(mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11) {
	    ed->focus = 1;
	    ed->cursor = textwidthx(str, mx-ed->x);
	} else
	    ed->focus = 0;
    }
    if(ed->focus && sdl_key) {
	if(ed->hide) {
	    for(i=0;ed->str[i];i++)
		echo[i] = 0x8D;
	    echo[i] = 0;
	    str = echo;
	} else
	    str = ed->str;

	l = strlen(ed->str);
	switch(sdl_key) {
	case SDLK_HOME:
	    ed->cursor = 0;
	    break;
	case SDLK_END:
	    ed->cursor = l;
	    break;
	case SDLK_LEFT:
	    if(ed->cursor > 0)
		ed->cursor --;
	    break;
	case SDLK_RIGHT:
	    if(ed->cursor < l)
		ed->cursor ++;
	    break;
	case SDLK_DELETE:
	    if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
		ed->str[ed->cursor] = 0;
	    else if(ed->cursor < l)
		memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
	    break;
	case SDLK_BACKSPACE:
	    if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)) {
		if(ed->cursor > 0)
		    memmove(ed->str, ed->str+ed->cursor, l-ed->cursor+1);
		ed->cursor = 0;
	    } else if(ed->cursor > 0) {
		ed->cursor--;
		memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
	    }
	    break;
	default:
#ifdef RAWINPUT
	    if(sdl_key>=SDLK_SPACE && sdl_key<=SDLK_z && l<255) {
		ch = sdl_key;
		if((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS))) {
		    if(ch>='a' && ch<='z')
			ch &= ~0x20;
		    p = strchr(shift_0, ch);
		    if(p)
			ch = shift_1[p-shift_0];
		}
		ts[0]=ed->hide?0x8D:ch;
		ts[1]=0;
		if(textwidth(str)+textwidth(ts) > ed->w-14)
		    break;
		memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
		ed->str[ed->cursor] = ch;
		ed->cursor++;
	    }
#else
	    if(sdl_ascii>=' ' && sdl_ascii<127) {
		ch = sdl_ascii;
		ts[0]=ed->hide?0x8D:ch;
		ts[1]=0;
		if(textwidth(str)+textwidth(ts) > ed->w-14)
		    break;
		memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
		ed->str[ed->cursor] = ch;
		ed->cursor++;
	    }
#endif
	    break;
	}
    }
}

void error_ui(pixel *vid_buf, int err, char *txt)
{
    int x0=(XRES-240)/2,y0=(YRES-60)/2,b=1,bq,mx,my;
    char *msg;

    msg = malloc(strlen(txt)+16);
    if(err)
	sprintf(msg, "%03d %s", err, txt);
    else
	sprintf(msg, "%s", txt);

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	clearrect(vid_buf, x0-2, y0-2, 244, 64);
	drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
	if(err)
	    drawtext(vid_buf, x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
	else
	    drawtext(vid_buf, x0+8, y0+8, "Error:", 255, 64, 32, 255);
	drawtext(vid_buf, x0+8, y0+26, msg, 255, 255, 255, 255);
	drawtext(vid_buf, x0+5, y0+49, "Dismiss", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	if(b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
	    break;

	if(sdl_key==SDLK_RETURN)
	    break;
	if(sdl_key==SDLK_ESCAPE)
	    break;
    }

    free(msg);

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }
}

void info_ui(pixel *vid_buf, char *top, char *txt)
{
    int x0=(XRES-240)/2,y0=(YRES-60)/2,b=1,bq,mx,my;

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	clearrect(vid_buf, x0-2, y0-2, 244, 64);
	drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, top, 160, 160, 255, 255);
	drawtext(vid_buf, x0+8, y0+26, txt, 255, 255, 255, 255);
	drawtext(vid_buf, x0+5, y0+49, "OK", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	if(b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
	    break;

	if(sdl_key==SDLK_RETURN)
	    break;
	if(sdl_key==SDLK_ESCAPE)
	    break;
    }

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }
}

void info_box(pixel *vid_buf, char *msg)
{
    int w = textwidth(msg)+16;
    int x0=(XRES-w)/2,y0=(YRES-24)/2;

    clearrect(vid_buf, x0-2, y0-2, w+4, 28);
    drawrect(vid_buf, x0, y0, w, 24, 192, 192, 192, 255);
    drawtext(vid_buf, x0+8, y0+8, msg, 192, 192, 240, 255);
    sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);
}

int confirm_ui(pixel *vid_buf, char *top, char *msg, char *btn)
{
    int x0=(XRES-240)/2,y0=(YRES-60)/2,b=1,bq,mx,my;
    int ret = 0;

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	clearrect(vid_buf, x0-2, y0-2, 244, 64);
	drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, top, 255, 216, 32, 255);
	drawtext(vid_buf, x0+8, y0+26, msg, 255, 255, 255, 255);
	drawtext(vid_buf, x0+5, y0+49, "Cancel", 255, 255, 255, 255);
	drawtext(vid_buf, x0+165, y0+49, btn, 255, 216, 32, 255);
	drawrect(vid_buf, x0, y0+44, 160, 16, 192, 192, 192, 255);
	drawrect(vid_buf, x0+160, y0+44, 80, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	if(b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+44 && my<=y0+60) {
	    ret = 1;
	    break;
	}
	if(b && !bq && mx>=x0 && mx<x0+160 && my>=y0+44 && my<=y0+60)
	    break;

	if(sdl_key==SDLK_RETURN) {
	    ret = 1;
	    break;
	}
	if(sdl_key==SDLK_ESCAPE)
	    break;
    }

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    return ret;
}

int execute_tagop(pixel *vid_buf, char *op, char *tag)
{
    int status;
    char *result;

    char *names[] = {"id", "tag", NULL};
    char *parts[2];

    char *uri = malloc(strlen(SERVER)+strlen(op)+32);
    sprintf(uri, "http://" SERVER "/powder/tag%s.php", op);

    parts[0] = svf_id;
    parts[1] = tag;

    result = http_multipart_post(
	uri,
	names, parts, NULL,
	svf_user, svf_pass,
	&status, NULL);

    free(uri);

    if(status!=200) {
	error_ui(vid_buf, status, http_ret_text(status));
	if(result)
	    free(result);
	return 1;
    }
    if(result && strncmp(result, "OK", 2)) {
	error_ui(vid_buf, 0, result);
	free(result);
	return 1;
    }

    if(result[2]) {
	strncpy(svf_tags, result+3, 255);
	svf_id[15] = 0;
    }

    if(result)
	free(result);

    return 0;
}

struct strlist {
    char *str;
    struct strlist *next;
};
void strlist_add(struct strlist **list, char *str)
{
    struct strlist *item = malloc(sizeof(struct strlist));
    item->str = mystrdup(str);
    item->next = *list;
    *list = item;
}
int strlist_find(struct strlist **list, char *str)
{
    struct strlist *item;
    for(item=*list;item;item=item->next)
	if(!strcmp(item->str, str))
	    return 1;
    return 0;
}
void strlist_free(struct strlist **list)
{
    struct strlist *item;
    while(*list) {
	item = *list;
	*list = (*list)->next;
	free(item);
    }
}

void tag_list_ui(pixel *vid_buf)
{
    int y,d,x0=(XRES-192)/2,y0=(YRES-256)/2,b=1,bq,mx,my,vp,vn;
    char *p,*q,s;
    char *tag=NULL, *op=NULL;
    ui_edit ed;
    struct strlist *vote=NULL,*down=NULL;

    ed.x = x0+25;
    ed.y = y0+221;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[new tag]";
    ed.focus = 0;
    ed.hide = 0;
    ed.cursor = 0;
    strcpy(ed.str, "");

    fillrect(vid_buf, -1, -1, XRES, YRES+60, 0, 0, 0, 192);
    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	op = tag = NULL;

	drawrect(vid_buf, x0, y0, 192, 256, 192, 192, 192, 255);
	clearrect(vid_buf, x0, y0, 192, 256);
	drawtext(vid_buf, x0+8, y0+8, "Current tags:", 255, 255, 255, 255);
	p = svf_tags;
	s = svf_tags[0] ? ' ' : 0;
	y = 36 + y0;
	while(s) {
	    q = strchr(p, ' ');
	    if(!q)
		q = p+strlen(p);
	    s = *q;
	    *q = 0;
	    if(svf_own || svf_admin) {
		drawtext(vid_buf, x0+20, y-1, "\x86", 160, 48, 32, 255);
		drawtext(vid_buf, x0+20, y-1, "\x85", 255, 255, 255, 255);
		d = 14;
		if(b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12) {
		    op = "delete";
		    tag = mystrdup(p);
		}
	    } else
		d = 0;
	    vp = strlist_find(&vote, p);
	    vn = strlist_find(&down, p);
	    if((!vp && !vn && !svf_own) || svf_admin) {
		drawtext(vid_buf, x0+d+20, y-1, "\x88", 32, 144, 32, 255);
		drawtext(vid_buf, x0+d+20, y-1, "\x87", 255, 255, 255, 255);
		if(b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12) {
		    op = "vote";
		    tag = mystrdup(p);
		    strlist_add(&vote, p);
		}
		drawtext(vid_buf, x0+d+34, y-1, "\x88", 144, 48, 32, 255);
		drawtext(vid_buf, x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
		if(b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12) {
		    op = "down";
		    tag = mystrdup(p);
		    strlist_add(&down, p);
		}
	    }
	    if(vp)
		drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted!", 48, 192, 48, 255);
	    if(vn)
		drawtext(vid_buf, x0+d+48+textwidth(p), y, " - voted.", 192, 64, 32, 255);
	    drawtext(vid_buf, x0+d+48, y, p, 192, 192, 192, 255);
	    *q = s;
	    p = q+1;
	    y += 16;
	}
	drawtext(vid_buf, x0+11, y0+219, "\x86", 32, 144, 32, 255);
	drawtext(vid_buf, x0+11, y0+219, "\x89", 255, 255, 255, 255);
	drawrect(vid_buf, x0+8, y0+216, 176, 16, 192, 192, 192, 255);
	ui_edit_draw(vid_buf, &ed);
	drawtext(vid_buf, x0+5, y0+245, "Close", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+240, 192, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	ui_edit_process(mx, my, b, &ed);

	if(b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
	    break;

	if(op) {
	    d = execute_tagop(vid_buf, op, tag);
	    free(tag);
	    op = tag = NULL;
	    if(d)
		goto finish;
	}

	if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232) {
	    d = execute_tagop(vid_buf, "add", ed.str);
	    strcpy(ed.str, "");
	    ed.cursor = 0;
	    if(d)
		goto finish;
	}

	if(sdl_key==SDLK_RETURN) {
	    if(!ed.focus)
		break;
	    d = execute_tagop(vid_buf, "add", ed.str);
	    strcpy(ed.str, "");
	    ed.cursor = 0;
	    if(d)
		goto finish;
	}
	if(sdl_key==SDLK_ESCAPE) {
	    if(!ed.focus)
		break;
	    strcpy(ed.str, "");
	    ed.cursor = 0;
	    ed.focus = 0;
	}
    }
    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }
    sdl_key = 0;

finish:
    strlist_free(&vote);
}

int save_name_ui(pixel *vid_buf)
{
    int x0=(XRES-192)/2,y0=(YRES-68-YRES/4)/2,b=1,bq,mx,my,ths,nd=0;
    void *th;
    ui_edit ed;

    th = build_thumb(&ths, 0);

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    ed.x = x0+25;
    ed.y = y0+25;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[simulation name]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(svf_name);
    strcpy(ed.str, svf_name);

    fillrect(vid_buf, -1, -1, XRES, YRES+60, 0, 0, 0, 192);
    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	drawrect(vid_buf, x0, y0, 192, 68+YRES/4, 192, 192, 192, 255);
	clearrect(vid_buf, x0, y0, 192, 68+YRES/4);
	drawtext(vid_buf, x0+8, y0+8, "New simulation name:", 255, 255, 255, 255);
	drawtext(vid_buf, x0+10, y0+23, "\x82", 192, 192, 192, 255);
	drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
	ui_edit_draw(vid_buf, &ed);
	drawrect(vid_buf, x0+(192-XRES/4)/2-2, y0+42, XRES/4+3, YRES/4+3, 128, 128, 128, 255);
	render_thumb(th, ths, 0, vid_buf, x0+(192-XRES/4)/2, y0+44, 4);
	drawtext(vid_buf, x0+5, y0+57+YRES/4, "Save simulation", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+52+YRES/4, 192, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	ui_edit_process(mx, my, b, &ed);

	if(b && !bq && ((mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36) ||
	                (mx>=x0 && mx<x0+192 && my>=y0+52+YRES/4 && my<y0+68+YRES/4))) {
	    free(th);
	    if(!ed.str[0])
		return 0;
	    nd = strcmp(svf_name, ed.str) || !svf_own;
	    strncpy(svf_name, ed.str, 63);
	    svf_name[63] = 0;
	    if(nd) {
		strcpy(svf_id, "");
		strcpy(svf_tags, "");
	    }
	    svf_open = 1;
	    svf_own = 1;
	    return nd+1;
	}

	if(sdl_key==SDLK_RETURN) {
	    free(th);
	    if(!ed.str[0])
		return 0;
	    nd = strcmp(svf_name, ed.str) || !svf_own;
	    strncpy(svf_name, ed.str, 63);
	    svf_name[63] = 0;
	    if(nd) {
		strcpy(svf_id, "");
		strcpy(svf_tags, "");
	    }
	    svf_open = 1;
	    svf_own = 1;
	    return nd+1;
	}
	if(sdl_key==SDLK_ESCAPE) {
	    if(!ed.focus)
		break;
	    ed.focus = 0;
	}
    }
    free(th);
    return 0;
}

void thumb_cache_inval(char *id);
void execute_save(pixel *vid_buf)
{
    int status;
    char *result;

    char *names[] = {"name", "data:save.bin", "thumb:thumb.bin", "id", NULL};
    char *parts[4];
    int plens[4];

    parts[0] = svf_name;
    plens[0] = strlen(svf_name);
    parts[1] = build_save(plens+1, 0, 0, XRES, YRES);
    parts[2] = build_thumb(plens+2, 1);
    if(svf_id[0]) {
	parts[3] = svf_id;
	plens[3] = strlen(svf_id);
    } else
	names[3] = NULL;

    result = http_multipart_post(
	"http://" SERVER "/powder/save.php",
	names, parts, plens,
	svf_user, svf_pass,
	&status, NULL);

    if(svf_last)
	free(svf_last);
    svf_last = parts[1];
    svf_lsize = plens[1];

    free(parts[2]);

    if(status!=200) {
	error_ui(vid_buf, status, http_ret_text(status));
	if(result)
	    free(result);
	return;
    }
    if(result && strncmp(result, "OK", 2)) {
	error_ui(vid_buf, 0, result);
	free(result);
	return;
    }

    if(result[2]) {
	strncpy(svf_id, result+3, 15);
	svf_id[15] = 0;
    }

    if(!svf_id[0]) {
	error_ui(vid_buf, 0, "No ID supplied by server");
	free(result);
	return;
    }

    thumb_cache_inval(svf_id);

    svf_own = 1;
    if(result)
	free(result);
}

void login_ui(pixel *vid_buf)
{
    int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq,mx,my,err;
    ui_edit ed1,ed2;
    char *res;

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    ed1.x = x0+25;
    ed1.y = y0+25;
    ed1.w = 158;
    ed1.nx = 1;
    ed1.def = "[user name]";
    ed1.focus = 1;
    ed1.hide = 0;
    ed1.cursor = strlen(svf_user);
    strcpy(ed1.str, svf_user);
    ed2.x = x0+25;
    ed2.y = y0+45;
    ed2.w = 158;
    ed2.nx = 1;
    ed2.def = "[password]";
    ed2.focus = 0;
    ed2.hide = 1;
    ed2.cursor = 0;
    strcpy(ed2.str, "");

    fillrect(vid_buf, -1, -1, XRES, YRES+60, 0, 0, 0, 192);
    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
	clearrect(vid_buf, x0, y0, 192, 80);
	drawtext(vid_buf, x0+8, y0+8, "Server login:", 255, 255, 255, 255);
	drawtext(vid_buf, x0+12, y0+23, "\x8B", 32, 64, 128, 255);
	drawtext(vid_buf, x0+12, y0+23, "\x8A", 255, 255, 255, 255);
	drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
	drawtext(vid_buf, x0+11, y0+44, "\x8C", 160, 144, 32, 255);
	drawtext(vid_buf, x0+11, y0+44, "\x84", 255, 255, 255, 255);
	drawrect(vid_buf, x0+8, y0+40, 176, 16, 192, 192, 192, 255);
	ui_edit_draw(vid_buf, &ed1);
	ui_edit_draw(vid_buf, &ed2);
	drawtext(vid_buf, x0+5, y0+69, "Sign in", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	ui_edit_process(mx, my, b, &ed1);
	ui_edit_process(mx, my, b, &ed2);

	if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
	    break;
	if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+42 && my<y0+46)
	    break;
	if(b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
	    break;

	if(sdl_key==SDLK_RETURN) {
	    if(!ed1.focus)
		break;
	    ed1.focus = 0;
	    ed2.focus = 1;
	}
	if(sdl_key==SDLK_ESCAPE) {
	    if(!ed1.focus && !ed2.focus)
		return;
	    ed1.focus = 0;
	    ed2.focus = 0;
	}
    }

    strcpy(svf_user, ed1.str);
    md5_ascii(svf_pass, (unsigned char *)ed2.str, 0);

    res = http_multipart_post(
	"http://" SERVER "/powder/login.php",
	NULL, NULL, NULL,
	svf_user, svf_pass,
	&err, NULL);
    if(err != 200) {
	error_ui(vid_buf, err, http_ret_text(err));
	if(res)
	    free(res);
	goto fail;
    }
    if(res && !strncmp(res, "OK", 2)) {
	if(!strcmp(res, "OK ADMIN"))
	    svf_admin = 1;
	else
	    svf_admin = 0;
	free(res);
	svf_login = 1;
	return;
    }
    if(!res)
	res = mystrdup("Unspecified Error");
    error_ui(vid_buf, 0, res);
    free(res);

fail:
    strcpy(svf_user, "");
    strcpy(svf_pass, "");
    svf_login = 0;
    svf_own = 0;
    svf_admin = 0;
}

void execute_delete(pixel *vid_buf, char *id)
{
    int status;
    char *result;

    char *names[] = {"id", NULL};
    char *parts[1];

    parts[0] = id;

    result = http_multipart_post(
	"http://" SERVER "/powder/delete.php",
	names, parts, NULL,
	svf_user, svf_pass,
	&status, NULL);

    if(status!=200) {
	error_ui(vid_buf, status, http_ret_text(status));
	if(result)
	    free(result);
	return;
    }
    if(result && strncmp(result, "OK", 2)) {
	error_ui(vid_buf, 0, result);
	free(result);
	return;
    }

    if(result)
	free(result);
}

static char hex[] = "0123456789ABCDEF";
void strcaturl(char *dst, char *src)
{
    char *d;
    unsigned char *s;

    for(d=dst; *d; d++) ;

    for(s=(unsigned char *)src; *s; s++) {
	if((*s>='0' && *s<='9') ||
	   (*s>='a' && *s<='z') ||
	   (*s>='A' && *s<='Z'))
	    *(d++) = *s;
	else {
	    *(d++) = '%';
	    *(d++) = hex[*s>>4];
	    *(d++) = hex[*s&15];
	}
    }
    *d = 0;
}

#define THUMB_CACHE_SIZE 256

char *thumb_cache_id[THUMB_CACHE_SIZE];
void *thumb_cache_data[THUMB_CACHE_SIZE];
int thumb_cache_size[THUMB_CACHE_SIZE];
int thumb_cache_lru[THUMB_CACHE_SIZE];

void thumb_cache_inval(char *id)
{
    int i,j;
    for(i=0;i<THUMB_CACHE_SIZE;i++)
	if(thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
	    break;
    if(i >= THUMB_CACHE_SIZE)
	return;
    free(thumb_cache_id[i]);
    free(thumb_cache_data[i]);
    thumb_cache_id[i] = NULL;
    for(j=0;j<THUMB_CACHE_SIZE;j++)
	if(thumb_cache_lru[j] > thumb_cache_lru[i])
	    thumb_cache_lru[j]--;
}
void thumb_cache_add(char *id, void *thumb, int size)
{
    int i,m=-1,j=-1;
    thumb_cache_inval(id);
    for(i=0;i<THUMB_CACHE_SIZE;i++) {
	if(!thumb_cache_id[i])
	    break;
	if(thumb_cache_lru[i] > m) {
	    m = thumb_cache_lru[i];
	    j = i;
	}
    }
    if(i >= THUMB_CACHE_SIZE) {
	thumb_cache_inval(thumb_cache_id[j]);
	i = j;
    }
    for(j=0;j<THUMB_CACHE_SIZE;j++)
	thumb_cache_lru[j] ++;
    thumb_cache_id[i] = mystrdup(id);
    thumb_cache_data[i] = malloc(size);
    memcpy(thumb_cache_data[i], thumb, size);
    thumb_cache_size[i] = size;
    thumb_cache_lru[i] = 0;
}
int thumb_cache_find(char *id, void **thumb, int *size)
{
    int i,j;
    for(i=0;i<THUMB_CACHE_SIZE;i++)
	if(thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
	    break;
    if(i >= THUMB_CACHE_SIZE)
	return 0;
    for(j=0;j<THUMB_CACHE_SIZE;j++)
	if(thumb_cache_lru[j] < thumb_cache_lru[i])
	    thumb_cache_lru[j]++;
    thumb_cache_lru[i] = 0;
    *thumb = malloc(thumb_cache_size[i]);
    *size = thumb_cache_size[i];
    memcpy(*thumb, thumb_cache_data[i], *size);
    return 1;
}

#define GRID_X 5
#define GRID_Y 4
#define GRID_P 3
#define GRID_S 6
#define GRID_Z 3

char *search_ids[GRID_X*GRID_Y];
int   search_votes[GRID_X*GRID_Y];
char *search_names[GRID_X*GRID_Y];
char *search_owners[GRID_X*GRID_Y];
void *search_thumbs[GRID_X*GRID_Y];
int   search_thsizes[GRID_X*GRID_Y];

#define TAG_MAX 256
char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int search_results(char *str, int votes)
{
    int i,j;
    char *p,*q,*r,*s;

    for(i=0;i<GRID_X*GRID_Y;i++) {
	if(search_ids[i]) {
	    free(search_ids[i]);
	    search_ids[i] = NULL;
	}
	if(search_names[i]) {
	    free(search_names[i]);
	    search_names[i] = NULL;
	}
	if(search_owners[i]) {
	    free(search_owners[i]);
	    search_owners[i] = NULL;
	}
	if(search_thumbs[i]) {
	    free(search_thumbs[i]);
	    search_thumbs[i] = NULL;
	    search_thsizes[i] = 0;
	}
    }
    for(j=0;j<TAG_MAX;j++)
	if(tag_names[j]) {
	    free(tag_names[j]);
	    tag_names[j] = NULL;
	}

    if(!str || !*str)
	return 0;

    i = 0;
    j = 0;
    s = NULL;
    while(1) {
	if(!*str)
	    break;
	p = strchr(str, '\n');
	if(!p)
	    p = str + strlen(str);
	else
	    *(p++) = 0;
	if(strncmp(str, "TAG ", 4)) {
	    if(i>=GRID_X*GRID_Y)
		break;
	    if(votes) {
		s = strchr(str, ' ');
		if(!s)
		    return i;
		*(s++) = 0;
		q = strchr(s, ' ');
	    } else
		q = strchr(str, ' ');
	    if(!q)
		return i;
	    *(q++) = 0;
	    r = strchr(q, ' ');
	    if(!r)
		return i;
	    *(r++) = 0;
	    search_ids[i] = mystrdup(str);
	    search_owners[i] = mystrdup(q);
	    search_names[i] = mystrdup(r);
	    if(s)
		search_votes[i] = atoi(s);
	    thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
	    i++;
	} else {
	    if(j >= TAG_MAX) {
		str = p;
		continue;
	    }
	    q = strchr(str+4, ' ');
	    if(!q) {
		str = p;
		continue;
	    }
	    *(q++) = 0;
	    tag_names[j] = mystrdup(str+4);
	    tag_votes[j] = atoi(q);
	    j++;
	}
	str = p;
    }
    if(*str)
	i++;
    return i;
}

#define IMGCONNS 3
#define TIMEOUT 100
#define HTTP_TIMEOUT 10

int search_own = 0;
int search_date = 0;
int search_page = 0;
char search_expr[256] = "";

int search_ui(pixel *vid_buf)
{
    int b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,own,last_own=search_own,page_count=0,last_page=0,last_date=0,j,w,h,st=0;
    int is_p1=0, exp_res=GRID_X*GRID_Y, tp, view_own=0, http_last_use=HTTP_TIMEOUT;
    ui_edit ed;

    void *http = NULL;
    int active = 0;
    char *last = NULL;
    int search = 0;
    int lasttime = TIMEOUT;
    char *uri;
    int status;
    char *results;
    char *tmp, ts[64];

    void *img_http[IMGCONNS];
    char *img_id[IMGCONNS];
    void *thumb, *data;
    int thlen, dlen;

    memset(img_http, 0, sizeof(img_http));
    memset(img_id, 0, sizeof(img_id));

    memset(search_ids, 0, sizeof(search_ids));
    memset(search_names, 0, sizeof(search_names));
    memset(search_owners, 0, sizeof(search_owners));
    memset(search_thumbs, 0, sizeof(search_thumbs));
    memset(search_thsizes, 0, sizeof(search_thsizes));

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    ed.x = 65;
    ed.y = 13;
    ed.w = XRES-200;
    ed.nx = 1;
    ed.def = "[search terms]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(search_expr);
    strcpy(ed.str, search_expr);

    sdl_wheel = 0;

    while(!sdl_poll()) {
	bq = b;
	mxq = mx;
	myq = my;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	if(mx!=mxq || my!=myq || sdl_wheel || b)
	    mmt = 0;
	else if(mmt<TIMEOUT)
	    mmt++;

	clearrect(vid_buf, -1, -1, XRES+1, YRES+61);

	drawtext(vid_buf, 11, 13, "Search:", 192, 192, 192, 255);
	if(!last || (!active && strcmp(last, ed.str)))
	    drawtext(vid_buf, 51, 11, "\x8E", 192, 160, 32, 255);
	else
	    drawtext(vid_buf, 51, 11, "\x8E", 32, 64, 160, 255);
	drawtext(vid_buf, 51, 11, "\x8F", 255, 255, 255, 255);
	drawrect(vid_buf, 48, 8, XRES-182, 16, 192, 192, 192, 255);

	if(!svf_login) {
	    search_own = 0;
	    drawrect(vid_buf, XRES-64, 8, 56, 16, 96, 96, 96, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x94", 96, 80, 16, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x93", 128, 128, 128, 255);
	    drawtext(vid_buf, XRES-46, 13, "My Own", 128, 128, 128, 255);
	} else if(search_own) {
	    fillrect(vid_buf, XRES-65, 7, 58, 18, 255, 255, 255, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x94", 192, 160, 64, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x93", 32, 32, 32, 255);
	    drawtext(vid_buf, XRES-46, 13, "My Own", 0, 0, 0, 255);
	} else {
	    drawrect(vid_buf, XRES-64, 8, 56, 16, 192, 192, 192, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x94", 192, 160, 32, 255);
	    drawtext(vid_buf, XRES-61, 11, "\x93", 255, 255, 255, 255);
	    drawtext(vid_buf, XRES-46, 13, "My Own", 255, 255, 255, 255);
	}

	if(search_date) {
	    fillrect(vid_buf, XRES-130, 7, 62, 18, 255, 255, 255, 255);
	    drawtext(vid_buf, XRES-126, 11, "\xA6", 32, 32, 32, 255);
	    drawtext(vid_buf, XRES-111, 13, "By date", 0, 0, 0, 255);
	} else {
	    drawrect(vid_buf, XRES-129, 8, 60, 16, 192, 192, 192, 255);
	    drawtext(vid_buf, XRES-126, 11, "\xA9", 144, 48, 32, 255);
	    drawtext(vid_buf, XRES-126, 11, "\xA8", 32, 144, 32, 255);
	    drawtext(vid_buf, XRES-126, 11, "\xA7", 255, 255, 255, 255);
	    drawtext(vid_buf, XRES-111, 13, "By votes", 255, 255, 255, 255);
	}

	if(search_page) {
	    drawtext(vid_buf, 4, YRES+44, "\x96", 255, 255, 255, 255);
	    drawrect(vid_buf, 1, YRES+40, 16, 16, 255, 255, 255, 255);
	}
	if(page_count > 9) {
	    drawtext(vid_buf, XRES-15, YRES+44, "\x95", 255, 255, 255, 255);
	    drawrect(vid_buf, XRES-18, YRES+40, 16, 16, 255, 255, 255, 255);
	}

	ui_edit_draw(vid_buf, &ed);

	tp = -1;
	if(is_p1) {
	    drawtext(vid_buf, (XRES-textwidth("Popular tags:"))/2, 31, "Popular tags:", 255, 192, 64, 255);
	    for(gj=0;gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14);gj++)
		for(gi=0;gi<GRID_X;gi++) {
		    pos = gi+GRID_X*gj;
		    if(pos>TAG_MAX || !tag_names[pos])
			break;
		    if(tag_votes[0])
			i = 127+(128*tag_votes[pos])/tag_votes[0];
		    else
			i = 192;
		    w = textwidth(tag_names[pos]);
		    if(w>XRES/GRID_X-5)
			w = XRES/GRID_X-5;
		    gx = (XRES/GRID_X)*gi;
		    gy = gj*14 + 46;
		    if(mx>=gx && mx<gx+(XRES/GRID_X) && my>=gy && my<gy+14) {
			j = (i*5)/6;
			tp = pos;
		    } else
			j = i;
		    drawtextmax(vid_buf, gx+(XRES/GRID_X-w)/2, gy, XRES/GRID_X-5, tag_names[pos], j, j, i, 255);
		}
	}

	mp = dp = -1;
	st = 0;
	for(gj=0;gj<GRID_Y;gj++)
	    for(gi=0;gi<GRID_X;gi++) {
		if(is_p1) {
		    pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
		    if(pos<0)
			break;
		} else 
		    pos = gi+GRID_X*gj;
		if(!search_ids[pos])
		    break;
		gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
		gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
		if(textwidth(search_names[pos]) > XRES/GRID_X-10) {
		    tmp = malloc(strlen(search_names[pos])+4);
		    strcpy(tmp, search_names[pos]);
		    j = textwidthx(tmp, XRES/GRID_X-15);
		    strcpy(tmp+j, "...");
		    drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(tmp)/2, gy+YRES/GRID_S+7, tmp, 192, 192, 192, 255);
		    free(tmp);
		} else
		    drawtext(vid_buf, gx+XRES/(GRID_S*2)-textwidth(search_names[pos])/2, gy+YRES/GRID_S+7, search_names[pos], 192, 192, 192, 255);
		j = textwidth(search_owners[pos]);
		if(mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
		   my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+31) {
		    st = 1;
		    drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
		} else
		    drawtext(vid_buf, gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
		if(search_thumbs[pos])
		    render_thumb(search_thumbs[pos], search_thsizes[pos], 1, vid_buf, gx, gy, GRID_S);
		own = svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin);
		if(mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy-2 && my<=gy+YRES/GRID_S+30)
		    mp = pos;
		if(own) {
		    if(mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4) {
			mp = -1;
			dp = pos;
		    }
		}
		if(mp==pos && !st)
		    drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
		else
		    drawrect(vid_buf, gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
		if(own) {
		    if(dp == pos)
			drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
		    else
			drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x86", 160, 48, 32, 255);
		    drawtext(vid_buf, gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
		}
		if(view_own || svf_admin) {
		    sprintf(ts+1, "%d", search_votes[pos]);
		    ts[0] = 0xBB;
		    for(j=1; ts[j]; j++)
			ts[j] = 0xBC;
		    ts[j-1] = 0xB9;
		    ts[j] = 0xBA;
		    ts[j+1] = 0;
		    w = gx+XRES/GRID_S-2-textwidth(ts);
		    h = gy+YRES/GRID_S-11;
		    drawtext(vid_buf, w, h, ts, 16, 72, 16, 255);
		    for(j=0; ts[j]; j++)
			ts[j] -= 14;
		    drawtext(vid_buf, w, h, ts, 192, 192, 192, 255);
		    sprintf(ts, "%d", search_votes[pos]);
		    for(j=0; ts[j]; j++)
			ts[j] += 127;
		    drawtext(vid_buf, w+3, h, ts, 255, 255, 255, 255);
		}
	    }

	if(mp!=-1 && mmt>=TIMEOUT/5 && !st) {
	    gi = mp % GRID_X;
	    gj = mp / GRID_X;
	    if(is_p1)
		gj += GRID_Y-GRID_P;
	    gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
	    gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
	    i = w = textwidth(search_names[mp]);
	    h = YRES/GRID_Z+30;
	    if(w<XRES/GRID_Z) w=XRES/GRID_Z;
	    gx += XRES/(GRID_S*2)-w/2;
	    gy += YRES/(GRID_S*2)-h/2;
	    if(gx<2) gx=2;
	    if(gx+w>=XRES-2) gx=XRES-3-w;
	    if(gy<32) gy=32;
	    if(gy+h>=YRES+58) gy=YRES+57-h;
	    clearrect(vid_buf, gx-2, gy-3, w+4, h);
	    drawrect(vid_buf, gx-2, gy-3, w+4, h, 160, 160, 192, 255);
	    if(search_thumbs[mp])
		render_thumb(search_thumbs[mp], search_thsizes[mp], 1, vid_buf, gx+(w-(XRES/GRID_Z))/2, gy, GRID_Z);
	    drawtext(vid_buf, gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
	    drawtext(vid_buf, gx+(w-textwidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
	}

	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	ui_edit_process(mx, my, b, &ed);

	if(sdl_key==SDLK_RETURN) {
	    if(!last || (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)))
		lasttime = TIMEOUT;
	    else if(search_ids[0] && !search_ids[1]) {
		bq = 0;
		b = 1;
		mp = 0;
	    }
	}
	if(sdl_key==SDLK_ESCAPE)
	    goto finish;

	if((b && !bq && mx>=1 && mx<=17 && my>=YRES+40 && my<YRES+56) || sdl_wheel>0) {
	    if(search_page) {
		search_page --;
		lasttime = TIMEOUT;
	    }
	    sdl_wheel = 0;
	}
	if((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+40 && my<YRES+56) || sdl_wheel<0) {
	    if(page_count>exp_res) {
		lasttime = TIMEOUT;
		search_page ++;
		page_count = exp_res;
	    }
	    sdl_wheel = 0;
	}

	if(b && !bq && mx>=XRES-64 && mx<=XRES-8 && my>=8 && my<=24 && svf_login) {
	    search_own = !search_own;
	    lasttime = TIMEOUT;
	}
	if(b && !bq && mx>=XRES-129 && mx<=XRES-65 && my>=8 && my<=24) {
	    search_date = !search_date;
	    lasttime = TIMEOUT;
	}

	if(b && !bq && dp!=-1)
	    if(confirm_ui(vid_buf, "Do you want to delete?", search_names[dp], "Delete")) {
		execute_delete(vid_buf, search_ids[dp]);
		lasttime = TIMEOUT;
		if(last) {
		    free(last);
		    last = NULL;
		}
	    }

	if(b && !bq && tp!=-1) {
	    strncpy(ed.str, tag_names[tp], 255);
	    lasttime = TIMEOUT;
	}

	if(b && !bq && mp!=-1 && st) {
	    sprintf(ed.str, "user:%s", search_owners[mp]);
	    lasttime = TIMEOUT;
	}

	if(b && !bq && mp!=-1 && !st) {
	    info_box(vid_buf, "Loading...");

	    uri = malloc(strlen(search_ids[mp])*3+strlen(SERVER)+64);
	    strcpy(uri, "http://" SERVER "/powder/saves/");
	    strcaturl(uri, search_ids[mp]);
	    data = http_simple_get(uri, &status, &dlen);
	    free(uri);

	    if(status == 200) {
		status = parse_save(data, dlen, 1, 0, 0);
		switch(status) {
		case 1:
		    error_ui(vid_buf, 0, "Simulation corrupted");
		    break;
		case 2:
		    error_ui(vid_buf, 0, "Simulation from a newer version");
		    break;
		case 3:
		    error_ui(vid_buf, 0, "Simulation on a too large grid");
		    break;
		}
		if(!status) {
		    if(svf_last)
			free(svf_last);
		    svf_last = data;
		    svf_lsize = dlen;

		    uri = malloc(strlen(search_ids[mp])*3+strlen(SERVER)+64);
		    strcpy(uri, "http://" SERVER "/powder/tags.php?id=");
		    strcaturl(uri, search_ids[mp]);
		    data = http_simple_get(uri, &status, NULL);
		    free(uri);

		    svf_open = 1;
		    svf_own = svf_login && !strcmp(search_owners[mp], svf_user);
		    strcpy(svf_id, search_ids[mp]);
		    strcpy(svf_name, search_names[mp]);
		    if(status == 200) {
			if(data) {
			    strncpy(svf_tags, data, 255);
			    svf_tags[255] = 0;
			} else
			    svf_tags[0] = 0;
		    } else
			svf_tags[0] = 0;
		} else {
		    svf_open = 0;
		    svf_own = 0;
		    svf_id[0] = 0;
		    svf_name[0] = 0;
		    svf_tags[0] = 0;
		    if(svf_last)
			free(svf_last);
		    svf_last = NULL;
		}
	    } else
		error_ui(vid_buf, status, http_ret_text(status));

	    if(data)
		free(data);
	    goto finish;
	}

	if(!last)
	    search = 1;
	else if(!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)) {
	    search = 1;
	    if(strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date) {
		search_page = 0;
		page_count = 0;
	    }
	    free(last);
	    last = NULL;
	} else
	    search = 0;

	if(search && lasttime>=TIMEOUT) {
	    lasttime = 0;
	    last = mystrdup(ed.str);
	    last_own = search_own;
	    last_date = search_date;
	    last_page = search_page;
	    active = 1;
	    uri = malloc(strlen(last)*3+80+strlen(SERVER)+strlen(svf_user));
	    if(search_own || svf_admin)
		tmp = "&v=1";
	    else
		tmp = "";
	    if(!search_own && !search_date && !*last) {
		if(search_page) {
		    exp_res = GRID_X*GRID_Y;
		    sprintf(uri, "http://" SERVER "/powder/search.php?s=%d&n=%d%s&q=", (search_page-1)*GRID_X*GRID_Y+GRID_X*GRID_P, exp_res+1, tmp);
		} else {
		    exp_res = GRID_X*GRID_P;
		    sprintf(uri, "http://" SERVER "/powder/search.php?s=%d&n=%d&t=%d%s&q=", 0, exp_res+1, ((GRID_Y-GRID_P)*YRES)/(GRID_Y*14)*GRID_X, tmp);
		}
	    } else {
		exp_res = GRID_X*GRID_Y;
		sprintf(uri, "http://" SERVER "/powder/search.php?s=%d&n=%d%s&q=", search_page*GRID_X*GRID_Y, exp_res+1, tmp);
	    }
	    strcaturl(uri, last);
	    if(search_own) {
		strcaturl(uri, " user:");
		strcaturl(uri, svf_user);
	    }
	    if(search_date)
		strcaturl(uri, " sort:date");
	    http = http_async_req_start(http, uri, NULL, 0, 1);
	    http_last_use = time(NULL);
	    free(uri);
	}

	if(active && http_async_req_status(http)) {
	    http_last_use = time(NULL);
	    results = http_async_req_stop(http, &status, NULL);
	    view_own = last_own;
	    if(status == 200)
		page_count = search_results(results, last_own||svf_admin);
	    is_p1 = (exp_res < GRID_X*GRID_Y);
	    free(results);
	    active = 0;
	}

	if(http && !active && (time(NULL)>http_last_use+HTTP_TIMEOUT)) {
	    http_async_req_close(http);
	    http = NULL;
	}

	for(i=0;i<IMGCONNS;i++) {
	    if(img_http[i] && http_async_req_status(img_http[i])) {
		thumb = http_async_req_stop(img_http[i], &status, &thlen);
		if(status != 200) {
		    if(thumb)
			free(thumb);
		    thumb = calloc(1,4);
		    thlen = 4;
		}
		thumb_cache_add(img_id[i], thumb, thlen);
		for(pos=0;pos<GRID_X*GRID_Y;pos++)
		    if(search_ids[pos] && !strcmp(search_ids[pos], img_id[i]))
			break;
		if(pos<GRID_X*GRID_Y) {
		    search_thumbs[pos] = thumb;
		    search_thsizes[pos] = thlen;
		} else
		    free(thumb);
		free(img_id[i]);
		img_id[i] = NULL;
	    }
	    if(!img_id[i]) {
		for(pos=0;pos<GRID_X*GRID_Y;pos++)
		    if(search_ids[pos] && !search_thumbs[pos]) {
			for(gi=0;gi<IMGCONNS;gi++)
			    if(img_id[gi] && !strcmp(search_ids[pos], img_id[gi]))
				break;
			if(gi<IMGCONNS)
			    continue;
			break;
		    }
		if(pos<GRID_X*GRID_Y) {
		    uri = malloc(strlen(search_ids[pos])*3+strlen(SERVER)+64);
		    strcpy(uri, "http://" SERVER "/powder/thumbs/");
		    strcaturl(uri, search_ids[pos]);
		    img_id[i] = mystrdup(search_ids[pos]);
		    img_http[i] = http_async_req_start(img_http[i], uri, NULL, 0, 1);
		    free(uri);
		}
	    }
	    if(!img_id[i] && img_http[i]) {
		http_async_req_close(img_http[i]);
		img_http[i] = NULL;
	    }
	}

	if(lasttime<TIMEOUT)
	    lasttime++;
    }

finish:
    if(last)
	free(last);
    if(http)
	http_async_req_close(http);
    for(i=0;i<IMGCONNS;i++)
	if(img_http[i])
	    http_async_req_close(img_http[i]);

    search_results("", 0);

    strcpy(search_expr, ed.str);

    return 0;
}

void draw_image(pixel *vid, pixel *img, int x, int y, int w, int h, int a)
{
    int i, j, r, g, b;
    for(j=0; j<h; j++)
	for(i=0; i<w; i++) {
	    r = PIXR(*img);
	    g = PIXG(*img);
	    b = PIXB(*img);
	    drawpixel(vid, x+i, y+j, r, g, b, a);
	    img++;
	}
}

void xor_rect(pixel *vid, int x, int y, int w, int h)
{
    int i;
    for(i=0; i<w; i+=2) {
	xor_pixel(x+i, y, vid);
	xor_pixel(x+i, y+h-1, vid);
    }
    for(i=2; i<h; i+=2) {
	xor_pixel(x, y+i, vid);
	xor_pixel(x+w-1, y+i, vid);
    }
}

int stamp_ui(pixel *vid_buf)
{
    int b=1,bq,mx,my,i,j,k,x,y,w,h,r=-1;

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	clearrect(vid_buf, -1, -1, XRES+1, YRES+61);
	k = 0;
	r = -1;
	for(j=0; j<STAMP_Y; j++)
	    for(i=0; i<STAMP_X; i++) {
		if(stamps[k].name[0] && stamps[k].thumb) {
		    x = (XRES*i)/STAMP_X + XRES/(STAMP_X*2);
		    y = (YRES*j)/STAMP_Y + YRES/(STAMP_Y*2);
		    w = stamps[k].thumb_w;
		    h = stamps[k].thumb_h;
		    x -= w/2;
		    y -= h/2;
		    draw_image(vid_buf, stamps[k].thumb, x, y, w, h, 255);
		    xor_rect(vid_buf, x, y, w, h);
		    if(mx>=x && mx<x+w && my>=y && my<y+h)
			r = k;
		}
		k++;
	    }
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	if(b==1)
	    break;
	if(b==4) {
	    r = -1;
	    break;
	}

	if(sdl_key==SDLK_RETURN)
	    break;
	if(sdl_key==SDLK_ESCAPE) {
	    r = -1;
	    break;
	}
    }

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    return r;
}

/***********************************************************
 *                      MESSAGE SIGNS                      *
 ***********************************************************/

void get_sign_pos(int i, int *x0, int *y0, int *w, int *h)
{
    *w = textwidth(signs[i].text) + 5;
    *h = 14;
    *x0 = (signs[i].ju == 2) ? signs[i].x - *w :
          (signs[i].ju == 1) ? signs[i].x - *w/2 : signs[i].x;
    *y0 = (signs[i].y > 18) ? signs[i].y - 18 : signs[i].y + 4;
}

void draw_icon(pixel *vid_buf, int x, int y, char ch, int flag)
{
    char t[2];
    t[0] = ch;
    t[1] = 0;
    if(flag) {
	fillrect(vid_buf, x-1, y-1, 17, 17, 255, 255, 255, 255);
	drawtext(vid_buf, x+3, y+2, t, 0, 0, 0, 255);
    } else {
	drawrect(vid_buf, x, y, 15, 15, 255, 255, 255, 255);
	drawtext(vid_buf, x+3, y+2, t, 255, 255, 255, 255);
    }
}

void render_signs(pixel *vid_buf)
{
    int i, j, x, y, w, h, dx, dy;
    for(i=0; i<MAXSIGNS; i++)
	if(signs[i].text[0]) {
	    get_sign_pos(i, &x, &y, &w, &h);
	    clearrect(vid_buf, x, y, w, h);
	    drawrect(vid_buf, x, y, w, h, 192, 192, 192, 255);
	    drawtext(vid_buf, x+3, y+3, signs[i].text, 255, 255, 255, 255);
	    x = signs[i].x;
	    y = signs[i].y;
	    dx = 1 - signs[i].ju;
	    dy = (signs[i].y > 18) ? -1 : 1;
	    for(j=0; j<4; j++) {
		drawpixel(vid_buf, x, y, 192, 192, 192, 255);
		x+=dx;
		y+=dy;
	    }
	}
}

void add_sign_ui(pixel *vid_buf, int mx, int my)
{
    int i, w, h, x, y, nm=0, ju;
    int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq;
    ui_edit ed;

    // check if it is an existing sign
    for(i=0; i<MAXSIGNS; i++)
	if(signs[i].text[0]) {
	    get_sign_pos(i, &x, &y, &w, &h);
	    if(mx>=x && mx<=x+w && my>=y && my<=y+h)
		break;
	}
    // else look for empty spot
    if(i >= MAXSIGNS) {
	nm = 1;
	for(i=0; i<MAXSIGNS; i++)
	    if(!signs[i].text[0])
		break;
    }
    if(i >= MAXSIGNS)
	return;

    if(nm) {
	signs[i].x = mx;
	signs[i].y = my;
	signs[i].ju = 1;
    }

    while(!sdl_poll()) {
	b = SDL_GetMouseState(&mx, &my);
	if(!b)
	    break;
    }

    ed.x = x0+25;
    ed.y = y0+25;
    ed.w = 158;
    ed.nx = 1;
    ed.def = "[message]";
    ed.focus = 1;
    ed.hide = 0;
    ed.cursor = strlen(signs[i].text);
    strcpy(ed.str, signs[i].text);
    ju = signs[i].ju;

    fillrect(vid_buf, -1, -1, XRES, YRES+60, 0, 0, 0, 192);
    while(!sdl_poll()) {
	bq = b;
	b = SDL_GetMouseState(&mx, &my);
	mx /= sdl_scale;
	my /= sdl_scale;

	drawrect(vid_buf, x0, y0, 192, 80, 192, 192, 192, 255);
	clearrect(vid_buf, x0, y0, 192, 80);
	drawtext(vid_buf, x0+8, y0+8, nm ? "New sign:" : "Edit sign:", 255, 255, 255, 255);
	drawtext(vid_buf, x0+12, y0+23, "\xA1", 32, 64, 128, 255);
	drawtext(vid_buf, x0+12, y0+23, "\xA0", 255, 255, 255, 255);
	drawrect(vid_buf, x0+8, y0+20, 176, 16, 192, 192, 192, 255);
	ui_edit_draw(vid_buf, &ed);
	drawtext(vid_buf, x0+8, y0+46, "Justify:", 255, 255, 255, 255);
	draw_icon(vid_buf, x0+50, y0+42, 0x9D, ju == 0);
	draw_icon(vid_buf, x0+68, y0+42, 0x9E, ju == 1);
	draw_icon(vid_buf, x0+86, y0+42, 0x9F, ju == 2);

	if(!nm) {
	    drawtext(vid_buf, x0+138, y0+45, "\x86", 160, 48, 32, 255);
	    drawtext(vid_buf, x0+138, y0+45, "\x85", 255, 255, 255, 255);
	    drawtext(vid_buf, x0+152, y0+46, "Delete", 255, 255, 255, 255);
	    drawrect(vid_buf, x0+134, y0+42, 50, 15, 255, 255, 255, 255);
	}

	drawtext(vid_buf, x0+5, y0+69, "OK", 255, 255, 255, 255);
	drawrect(vid_buf, x0, y0+64, 192, 16, 192, 192, 192, 255);

	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);

	ui_edit_process(mx, my, b, &ed);

	if(b && !bq && mx>=x0+50 && mx<=x0+67 && my>=y0+42 && my<=y0+59)
	    ju = 0;
	if(b && !bq && mx>=x0+68 && mx<=x0+85 && my>=y0+42 && my<=y0+59)
	    ju = 1;
	if(b && !bq && mx>=x0+86 && mx<=x0+103 && my>=y0+42 && my<=y0+59)
	    ju = 2;

	if(b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
	    break;
	if(b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
	    break;

	if(!nm && b && !bq && mx>=x0+134 && my>=y0+42 && mx<=x0+184 && my<=y0+59) {
	    signs[i].text[0] = 0;
	    return;
	}

	if(sdl_key==SDLK_RETURN)
	    break;
	if(sdl_key==SDLK_ESCAPE) {
	    if(!ed.focus)
		return;
	    ed.focus = 0;
	}
    }

    strcpy(signs[i].text, ed.str);
    signs[i].ju = ju;
}

/***********************************************************
 *                       CONFIG FILE                       *
 ***********************************************************/

char http_proxy[256] = "";

void save_string(FILE *f, char *str)
{
    int li = strlen(str);
    unsigned char lb[2];
    lb[0] = li;
    lb[1] = li >> 8;
    fwrite(lb, 2, 1, f);
    fwrite(str, li, 1, f);
}

int load_string(FILE *f, char *str, int max)
{
    int li;
    unsigned char lb[2];
    fread(lb, 2, 1, f);
    li = lb[0] | (lb[1] << 8);
    if(li > max) {
	str[0] = 0;
	return 1;
    }
    fread(str, li, 1, f);
    str[li] = 0;
    return 0;
}

unsigned char last_major=0, last_minor=0, update_flag=0;

void save_presets(int do_update)
{
    FILE *f=fopen("powder.def", "wb");
    unsigned char sig[4] = {0x4D, 0x6F, 0x46, 0x6F};
    unsigned char tmp = sdl_scale;
    if(!f)
	return;
    fwrite(sig, 1, 4, f);
    save_string(f, svf_user);
    save_string(f, svf_pass);
    fwrite(&tmp, 1, 1, f);
    tmp = cmode;
    fwrite(&tmp, 1, 1, f);
    tmp = svf_admin;
    fwrite(&tmp, 1, 1, f);
    save_string(f, http_proxy);
    tmp = SAVE_VERSION;
    fwrite(&tmp, 1, 1, f);
    tmp = MINOR_VERSION;
    fwrite(&tmp, 1, 1, f);
    tmp = do_update;
    fwrite(&tmp, 1, 1, f);
    fclose(f);
}

void load_presets(void)
{
    FILE *f=fopen("powder.def", "rb");
    unsigned char sig[4], tmp;
    if(!f)
	return;
    fread(sig, 1, 4, f);
    if(sig[0]!=0x4D || sig[1]!=0x6F || sig[2]!=0x46 || sig[3]!=0x6F)
	return;
    if(load_string(f, svf_user, 63))
	goto fail;
    if(load_string(f, svf_pass, 63))
	goto fail;
    svf_login = !!svf_user[0];
    if(fread(&tmp, 1, 1, f) != 1)
	goto fail;
    sdl_scale = (tmp == 2) ? 2 : 1;
    if(fread(&tmp, 1, 1, f) != 1)
	goto fail;
    cmode = tmp%4;
    if(fread(&tmp, 1, 1, f) != 1)
	goto fail;
    svf_admin = tmp;
    if(load_string(f, http_proxy, 255))
	goto fail;
    if(fread(sig, 1, 3, f) != 3)
	goto fail;
    last_major = sig[0];
    last_minor = sig[1];
    update_flag = sig[2];
fail:
    fclose(f);
}

void dim_copy(pixel *dst, pixel *src)
{
    int i,r,g,b;
    for(i=0;i<XRES*YRES;i++) {
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

unsigned int fire_alpha[CELL*3][CELL*3];
void prepare_alpha(void)
{
    int x,y,i,j;
    float temp[CELL*3][CELL*3];
    memset(temp, 0, sizeof(temp));
    for(x=0; x<CELL; x++)
	for(y=0; y<CELL; y++)
	    for(i=-CELL;i<CELL;i++)
		for(j=-CELL;j<CELL;j++)
		    temp[y+CELL+j][x+CELL+i] += exp(-0.1*(i*i+j*j));
    for(x=0; x<CELL*3; x++)
	for(y=0; y<CELL*3; y++)
	    fire_alpha[y][x] = (int)(255.0f*temp[y][x]/(CELL*CELL));
}

void render_fire(pixel *dst)
{
    int i,j,x,y,r,g,b;
    for(j=0;j<YRES/CELL;j++)
	for(i=0;i<XRES/CELL;i++) {
	    r = fire_r[j][i];
	    g = fire_g[j][i];
	    b = fire_b[j][i];
	    if(r || g || b)
		for(y=-CELL+1;y<2*CELL;y++)
		    for(x=-CELL+1;x<2*CELL;x++)
			addpixel(dst, i*CELL+x, j*CELL+y, r, g, b, fire_alpha[y+CELL][x+CELL]);
	    for(y=-1;y<2;y++)
		for(x=-1;x<2;x++)
		    if(i+x>=0 && j+y>=0 && i+x<XRES/CELL && j+y<YRES/CELL && (x || y)) {
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

int zoom_en = 0;
int zoom_x=(XRES-ZSIZE)/2, zoom_y=(YRES-ZSIZE)/2;
int zoom_wx=0, zoom_wy=0;
void render_zoom(pixel *img)
{
    int x, y, i, j;
    pixel pix;
    drawrect(img, zoom_wx-2, zoom_wy-2, ZSIZE*ZFACTOR+2, ZSIZE*ZFACTOR+2, 192, 192, 192, 255);
    drawrect(img, zoom_wx-1, zoom_wy-1, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR, 0, 0, 0, 255);
    clearrect(img, zoom_wx, zoom_wy, ZSIZE*ZFACTOR, ZSIZE*ZFACTOR);
    for(j=0; j<ZSIZE; j++)
	for(i=0; i<ZSIZE; i++) {
	    pix = img[(j+zoom_y)*XRES+(i+zoom_x)];
	    for(y=0; y<ZFACTOR-1; y++)
		for(x=0; x<ZFACTOR-1; x++)
		    img[(j*ZFACTOR+y+zoom_wy)*XRES+(i*ZFACTOR+x+zoom_wx)] = pix;
	}
    if(zoom_en) {
	for(j=-1; j<=ZSIZE; j++) {
	    xor_pixel(zoom_x+j, zoom_y-1, img);
	    xor_pixel(zoom_x+j, zoom_y+ZSIZE, img);
	}
	for(j=0; j<ZSIZE; j++) {
	    xor_pixel(zoom_x-1, zoom_y+j, img);
	    xor_pixel(zoom_x+ZSIZE, zoom_y+j, img);
	}
    }
}

void render_cursor(pixel *vid, int x, int y, int t, int r)
{
    int i,j;
    if(t<PT_NUM) {
	if(r<=0)
	    xor_pixel(x, y, vid);
	else
	    for(j=0; j<=r; j++)
		for(i=0; i<=r; i++)
		    if(i*i+j*j<=r*r && ((i+1)*(i+1)+j*j>r*r || i*i+(j+1)*(j+1)>r*r)) {
			xor_pixel(x+i, y+j, vid);
			if(j) xor_pixel(x+i, y-j, vid);
			if(i) xor_pixel(x-i, y+j, vid);
			if(i&&j) xor_pixel(x-i, y-j, vid);
		    }
    } else {
	x = (x/CELL) * CELL;
	y = (y/CELL) * CELL;
	for(i=0; i<CELL; i++) {
	    xor_pixel(x+i, y, vid);
	    xor_pixel(x+i, y+CELL-1, vid);
	}
	for(i=1; i<CELL-1; i++) {
	    xor_pixel(x, y+i, vid);
	    xor_pixel(x+CELL-1, y+i, vid);
	}
    }
}

#define x86_cpuid(func,ax,bx,cx,dx) \
    __asm__ __volatile__ ("cpuid": \
    "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func));

int cpu_check(void)
{
#ifdef X86
    unsigned ax,bx,cx,dx;
    x86_cpuid(0, ax, bx, cx, dx);
    if(bx==0x68747541 && cx==0x444D4163 && dx==0x69746E65)
	amd = 1;
    x86_cpuid(1, ax, bx, cx, dx);
#ifdef X86_SSE
    if(!(dx&(1<<25)))
	return 1;
#endif
#ifdef X86_SSE2
    if(!(dx&(1<<26)))
	return 1;
#endif
#ifdef X86_SSE3
    if(!(cx&1))
	return 1;
#endif
#endif
    return 0;
}

char *tag = "(c) 2008-9 Stanislaw Skowronek";
int itc = 0;
char itc_msg[64] = "[?]";

pixel *fire_bg;
void set_cmode(int cm)
{
    cmode = cm;
    itc = 51;
    if(cmode==3) {
	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));
	strcpy(itc_msg, "Fire Display");
    } else if(cmode==2) {
	memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
	strcpy(itc_msg, "Persistent Display");
    } else if(cmode==1)
	strcpy(itc_msg, "Pressure Display");
    else
	strcpy(itc_msg, "Velocity Display");
}

char my_uri[] = "http://" SERVER "/update/powder-"
#if defined WIN32
                "win32"
#elif defined LIN32
                "lin32"
#elif defined LIN64
                "lin64"
#elif defined MACOSX
                "macosx"
#else
                "unknown"
#endif
#ifdef MACOSX
		".bin"
#else
#if defined X86_SSE3
		"-sse3.bin"
#elif defined X86_SSE2
		"-sse2.bin"
#elif defined X86_SSE
		"-sse.bin"
#else
		".bin"
#endif
#endif
		;

char *download_ui(pixel *vid_buf, char *uri, int *len)
{
    void *http = http_async_req_start(NULL, uri, NULL, 0, 0);
    int x0=(XRES-240)/2,y0=(YRES-60)/2;
    int done, total, i, ret, zlen, ulen;
    char str[16], *tmp, *res;

    while(!http_async_req_status(http)) {
	sdl_poll();

	http_async_get_length(http, &total, &done);

	clearrect(vid_buf, x0-2, y0-2, 244, 64);
	drawrect(vid_buf, x0, y0, 240, 60, 192, 192, 192, 255);
	drawtext(vid_buf, x0+8, y0+8, "Please wait", 255, 216, 32, 255);
	drawtext(vid_buf, x0+8, y0+26, "Downloading update...", 255, 255, 255, 255);

	if(total) {
	    i = (236*done)/total;
	    fillrect(vid_buf, x0+1, y0+45, i+1, 14, 255, 216, 32, 255);
	    i = (100*done)/total;
	    sprintf(str, "%d%%", i);
	    if(i<50)
		drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 192, 192, 192, 255);
	    else
		drawtext(vid_buf, x0+120-textwidth(str)/2, y0+48, str, 0, 0, 0, 255);
	} else
	    drawtext(vid_buf, x0+120-textwidth("Waiting...")/2, y0+48, "Waiting...", 255, 216, 32, 255);

	drawrect(vid_buf, x0, y0+44, 240, 16, 192, 192, 192, 255);
	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);
    }

    tmp = http_async_req_stop(http, &ret, &zlen);
    if(ret!=200) {
	error_ui(vid_buf, ret, http_ret_text(ret));
	if(tmp)
	    free(tmp);
	return NULL;
    }
    if(!tmp) {
	error_ui(vid_buf, 0, "Server did not return data");
	return NULL;
    }

    if(zlen<16)
	goto corrupt;
    if(tmp[0]!=0x42 || tmp[1]!=0x75 || tmp[2]!=0x54 || tmp[3]!=0x54)
	goto corrupt;

    ulen  = (unsigned char)tmp[4];
    ulen |= ((unsigned char)tmp[5])<<8;
    ulen |= ((unsigned char)tmp[6])<<16;
    ulen |= ((unsigned char)tmp[7])<<24;

    res = malloc(ulen);
    if(!res)
	goto corrupt;

    if(BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&ulen, (char *)(tmp+8), zlen-8, 0, 0)) {
	free(res);
	goto corrupt;
    }

    free(tmp);
    if(len)
	*len = ulen;
    return res;

corrupt:
    error_ui(vid_buf, 0, "Downloaded update is corrupted");
    free(tmp);
    return NULL;
}

int main(int argc, char *argv[])
{
    pixel *vid_buf=calloc(XRES*(YRES+60), PIXELSIZE);
    void *http_ver_check;
    char *ver_data=NULL, *tmp;
    int i, j, bq, fire_fc=0, do_check=0, old_version=0, http_ret=0, major, minor, old_ver_len;
#ifdef INTERNAL
    int vs = 0;
#endif
    int x, y, b = 0, sl=1, sr=0, su=0, c, lb = 0, lx = 0, ly = 0, lm = 0, tx, ty;
    int da = 0, db = 0, it = 2047, mx, my, bs = 2;
    float nfvx, nfvy;
    int load_mode=0, load_w=0, load_h=0, load_x=0, load_y=0, load_size=0;
    void *load_data=NULL;
    pixel *load_img=NULL, *fbi_img=NULL;
    int save_mode=0, save_x=0, save_y=0, save_w=0, save_h=0;

    old_ver_len = textwidth(old_ver_msg);

    parts = calloc(sizeof(particle), NPART);
    for(i=0; i<NPART-1; i++)
	parts[i].life = i+1;
    parts[NPART-1].life = -1;
    pfree = 0;
    fire_bg=calloc(XRES*YRES, PIXELSIZE);
    memset(signs, 0, sizeof(signs));

    fbi_img = render_packed_rgb(fbi, FBI_W, FBI_H, FBI_CMP);

    load_presets();

    for(i=1; i<argc; i++) {
	if(!strncmp(argv[i], "scale:", 6)) {
	    sdl_scale = (argv[i][6]=='2') ? 2 : 1;
	} else if(!strncmp(argv[i], "proxy:", 6)) {
	    memset(http_proxy, 0, sizeof(http_proxy));
	    strncpy(http_proxy, argv[i]+6, 255);
	}
    }

    save_presets(0);

    make_kernel();
    prepare_alpha();

    stamp_init();

    sdl_open();
    http_init(http_proxy[0] ? http_proxy : NULL);

    if(cpu_check()) {
	error_ui(vid_buf, 0, "Unsupported CPU. Try another version.");
	return 1;
    }

    http_ver_check = http_async_req_start(NULL, "http://" SERVER "/powder/version.txt", NULL, 0, 0);

    while(!sdl_poll()) {
	for(i=0; i<YRES/CELL; i++) {
	    pv[i][0] = pv[i][0]*0.8f;
	    pv[i][1] = pv[i][1]*0.8f;
	    pv[i][2] = pv[i][2]*0.8f;
	    pv[i][XRES/CELL-2] = pv[i][XRES/CELL-2]*0.8f;
	    pv[i][XRES/CELL-1] = pv[i][XRES/CELL-1]*0.8f;
	    vx[i][0] = vx[i][1]*0.9f;
	    vx[i][1] = vx[i][2]*0.9f;
	    vx[i][XRES/CELL-2] = vx[i][XRES/CELL-3]*0.9f;
	    vx[i][XRES/CELL-1] = vx[i][XRES/CELL-2]*0.9f;
	    vy[i][0] = vy[i][1]*0.9f;
	    vy[i][1] = vy[i][2]*0.9f;
	    vy[i][XRES/CELL-2] = vy[i][XRES/CELL-3]*0.9f;
	    vy[i][XRES/CELL-1] = vy[i][XRES/CELL-2]*0.9f;
	}
	for(i=0; i<XRES/CELL; i++) {
	    pv[0][i] = pv[0][i]*0.8f;
	    pv[1][i] = pv[1][i]*0.8f;
	    pv[2][i] = pv[2][i]*0.8f;
	    pv[YRES/CELL-2][i] = pv[YRES/CELL-2][i]*0.8f;
	    pv[YRES/CELL-1][i] = pv[YRES/CELL-1][i]*0.8f;
	    vx[0][i] = vx[1][i]*0.9f;
	    vx[1][i] = vx[2][i]*0.9f;
	    vx[YRES/CELL-2][i] = vx[YRES/CELL-3][i]*0.9f;
	    vx[YRES/CELL-1][i] = vx[YRES/CELL-2][i]*0.9f;
	    vy[0][i] = vy[1][i]*0.9f;
	    vy[1][i] = vy[2][i]*0.9f;
	    vy[YRES/CELL-2][i] = vy[YRES/CELL-3][i]*0.9f;
	    vy[YRES/CELL-1][i] = vy[YRES/CELL-2][i]*0.9f;
	}

	for(j=1; j<YRES/CELL; j++)
	    for(i=1; i<XRES/CELL; i++)
		if(bmap[j][i]==1 || bmap[j][i]==8 || (bmap[j][i]==7 && !emap[j][i])) {
		    vx[j][i] = 0.0f;
		    vx[j][i-1] = 0.0f;
		    vy[j][i] = 0.0f;
		    vy[j-1][i] = 0.0f;
		}

	if(!sys_pause)
	    update_air();
	if(cmode==0 || cmode==1)
	    draw_air(vid_buf);
	else if(cmode==2)
	    memcpy(vid_buf, fire_bg, XRES*YRES*PIXELSIZE);
	else
	    memset(vid_buf, 0, XRES*YRES*PIXELSIZE);
	update_particles(vid_buf);

	if(cmode==2) {
	    if(!fire_fc)
		dim_copy(fire_bg, vid_buf);
	    else
		memcpy(fire_bg, vid_buf, XRES*YRES*PIXELSIZE);
	    fire_fc = (fire_fc+1) % 3;
	}
	if(cmode==3)
	    render_fire(vid_buf);

	render_signs(vid_buf);

	memset(vid_buf+(XRES*YRES), 0, PIXELSIZE*XRES*60);
	for(i=0; i<PT_NUM; i++)
	    draw_tool(vid_buf, i, sl, sr, pcolors[i]);
	draw_tool(vid_buf, 22, sl, sr, PIXPACK(0xC0C0C0));
	draw_tool(vid_buf, 23, sl, sr, PIXPACK(0x808080));
	draw_tool(vid_buf, 24, sl, sr, PIXPACK(0xFF8080));
	draw_tool(vid_buf, 25, sl, sr, PIXPACK(0x808080));
	draw_tool(vid_buf, 26, sl, sr, PIXPACK(0x808080));
	draw_tool(vid_buf, 27, sl, sr, PIXPACK(0x8080FF));
	draw_tool(vid_buf, 28, sl, sr, PIXPACK(0xC0C0C0));
	draw_tool(vid_buf, 29, sl, sr, PIXPACK(0x808080));
	draw_tool(vid_buf, 30, sl, sr, PIXPACK(0x808080));
	draw_tool(vid_buf, 31, sl, sr, PIXPACK(0x808080));

	draw_svf_ui(vid_buf);

	if(http_ver_check) {
	    if(!do_check && http_async_req_status(http_ver_check)) {
		ver_data = http_async_req_stop(http_ver_check, &http_ret, NULL);
		if(http_ret==200 && ver_data) {
		    if(sscanf(ver_data, "%d.%d", &major, &minor)==2)
			if(major>SAVE_VERSION || (major==SAVE_VERSION && minor>MINOR_VERSION))
			    old_version = 1;
		    free(ver_data);
		}
		http_ver_check = NULL;
	    }
	    do_check = (do_check+1) & 15;
	}

	if(sdl_key=='q' || sdl_key==SDLK_ESCAPE)
	    break;
	if((sdl_key=='l' || sdl_key=='k') && stamps[0].name[0]) {
	    if(load_mode) {
		free(load_img);
		free(load_data);
		load_mode = 0;
		load_data = NULL;
		load_img = NULL;
	    }
	    if(it > 50)
		it = 50;
	    if(sdl_key=='k' && stamps[1].name[0]) {
		j = stamp_ui(vid_buf);
		if(j>=0)
		    load_data = stamp_load(j, &load_size);
		else
		    load_data = NULL;
	    } else
		load_data = stamp_load(0, &load_size);
	    if(load_data) {
		load_img = prerender_save(load_data, load_size, &load_w, &load_h);
		if(load_img)
		    load_mode = 1;
		else
		    free(load_data);
	    }
	}
	if(sdl_key=='s') {
	    if(it > 50)
		it = 50;
	    save_mode = 1;
	}
	if(sdl_key==SDLK_SPACE)
	    sys_pause = !sys_pause;
	if(sdl_key=='p')
	    dump_frame(vid_buf, XRES, YRES, XRES);
	if(sdl_key=='c') {
	    set_cmode((cmode+1) % 4);
	    if(it > 50)
		it = 50;
	}
#ifdef INTERNAL
	if(sdl_key=='v')
	    vs = !vs;
	if(vs)
	    dump_frame(vid_buf, XRES, YRES, XRES);
#endif

	if(sdl_wheel) {
	    bs += sdl_wheel;
	    if(bs>6)
		bs = 6;
	    if(bs<0)
		bs = 0;
	    sdl_wheel = 0;
	    if(su >= PT_NUM) {
		if(sl < PT_NUM)
		    su = sl;
		if(sr < PT_NUM)
		    su = sr;
	    }
	}

	bq = b;
	b = SDL_GetMouseState(&x, &y);

	if(zoom_en && x>=sdl_scale*zoom_wx && y>=sdl_scale*zoom_wy
	           && x<sdl_scale*(zoom_wx+ZFACTOR*ZSIZE)
		   && y<sdl_scale*(zoom_wy+ZFACTOR*ZSIZE)) {
	    x = (((x/sdl_scale-zoom_wx)/ZFACTOR)+zoom_x)*sdl_scale;
	    y = (((y/sdl_scale-zoom_wy)/ZFACTOR)+zoom_y)*sdl_scale;
	}
	mx = x;
	my = y;

	if(update_flag) {
	    info_box(vid_buf, "Finalizing update...");
	    if(last_major>SAVE_VERSION || (last_major==SAVE_VERSION && last_minor>=MINOR_VERSION)) {
		update_cleanup();
		error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
	    } else {
		if(update_finish())
		    error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
		else
		    info_ui(vid_buf, "Update success", "You have successfully updated the Powder Toy!");
	    }
	    update_flag = 0;
	}

	if(b && !bq && x>=(512-19-old_ver_len)*sdl_scale &&
	   x<=(512-14)*sdl_scale && y>=(YRES-22)*sdl_scale && y<=(YRES-9)*sdl_scale && old_version) {
	    tmp = malloc(64);
	    sprintf(tmp, "Your version: %d.%d, new version: %d.%d.", SAVE_VERSION, MINOR_VERSION, major, minor);
	    if(confirm_ui(vid_buf, "Do you want to update The Powder Toy?", tmp, "Update")) {
		free(tmp);
		tmp = download_ui(vid_buf, my_uri, &i);
		if(tmp) {
		    save_presets(1);
		    if(update_start(tmp, i)) {
			update_cleanup();
			save_presets(0);
			error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
		    } else
			return 0;
		}
	    } else
		free(tmp);
	}

	if(y>=sdl_scale*YRES && y<sdl_scale*(YRES+40)) {
	    tx = (x/sdl_scale)/32;
	    ty = ((y/sdl_scale)-YRES)/20;
	    c = 2*tx+ty;
	    if((c>=0 && c<PT_NUM) || (c>=22 && c<32)) {
		db = c;
		if(da < 51)
		    da ++;
	    } else {
		if(da > 0)
		    da --;
	    }
	} else if(y>=sdl_scale*(YRES+40)) {
	    if(x>=189*sdl_scale && x<=365*sdl_scale && svf_login && svf_open) {
		db = svf_own ? 257 : 256;
		if(da < 51)
		    da ++;
	    } else if(x>=367*sdl_scale && x<383*sdl_scale) {
		db = 266;
		if(da < 51)
		    da ++;
	    } else if(x>=37*sdl_scale && x<=187*sdl_scale && svf_login) {
		db = 259;
		if(svf_open && svf_own && x<=55*sdl_scale)
		    db = 258;
		if(da < 51)
		    da ++;
	    } else if(x>=385*sdl_scale && x<=476*sdl_scale) {
		db = svf_login ? 261 : 260;
		if(svf_admin)
		    db = 268;
		if(da < 51)
		    da ++;
	    } else if(x>=sdl_scale && x<=17*sdl_scale) {
		db = 262;
		if(da < 51)
		    da ++;
	    } else if(x>=494*sdl_scale && x<=509*sdl_scale) {
		db = sys_pause ? 264 : 263;
		if(da < 51)
		    da ++;
	    } else if(x>=476*sdl_scale && x<=491*sdl_scale) {
		db = 267;
		if(da < 51)
		    da ++;
	    } else if(x>=19*sdl_scale && x<=35*sdl_scale && svf_open) {
		db = 265;
		if(da < 51)
		    da ++;
	    } else if(da > 0)
		da --;
	} else
	    if(da > 0)
		da --;

	if(!sdl_zoom_trig && zoom_en==1)
	    zoom_en = 0;

	if(sdl_key==Z_keysym && zoom_en==2)
	    zoom_en = 1;

	if(load_mode) {
	    load_x = CELL*((mx/sdl_scale-load_w/2+CELL/2)/CELL);
	    load_y = CELL*((my/sdl_scale-load_h/2+CELL/2)/CELL);
	    if(load_x+load_w>XRES) load_x=XRES-load_w;
	    if(load_y+load_h>YRES) load_y=YRES-load_h;
	    if(load_x<0) load_x=0;
	    if(load_y<0) load_y=0;
	    if(bq==1 && !b) {
		parse_save(load_data, load_size, 0, load_x, load_y);
		free(load_data);
		free(load_img);
		load_mode = 0;
	    } else if(bq==4 && !b) {
		free(load_data);
		free(load_img);
		load_mode = 0;
	    }
	} else if(save_mode==1) {
	    save_x = (mx/sdl_scale)/CELL;
	    save_y = (my/sdl_scale)/CELL;
	    if(save_x >= XRES/CELL) save_x = XRES/CELL-1;
	    if(save_y >= YRES/CELL) save_y = YRES/CELL-1;
	    save_w = 1;
	    save_h = 1;
	    if(b==1)
		save_mode = 2;
	    else if(b==4)
		save_mode = 0;
	} else if(save_mode==2) {
	    save_w = (mx/sdl_scale+CELL/2)/CELL - save_x;
	    save_h = (my/sdl_scale+CELL/2)/CELL - save_y;
	    if(save_w>XRES/CELL) save_w = XRES/CELL;
	    if(save_h>YRES/CELL) save_h = YRES/CELL;
	    if(save_w<1) save_w = 1;
	    if(save_h<1) save_h = 1;
	    if(!b) {
		stamp_save(save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
		save_mode = 0;
	    }
	} else if(sdl_zoom_trig && zoom_en<2) {
	    x /= sdl_scale;
	    y /= sdl_scale;
	    x -= ZSIZE/2;
	    y -= ZSIZE/2;
	    if(x<0) x=0;
	    if(y<0) y=0;
	    if(x>XRES-ZSIZE) x=XRES-ZSIZE;
	    if(y>YRES-ZSIZE) y=YRES-ZSIZE;
	    zoom_x = x;
	    zoom_y = y;
	    zoom_wx = (x<XRES/2) ? XRES-ZSIZE*ZFACTOR : 0;
	    zoom_wy = 0;
	    zoom_en = 1;
	    if(!b && bq)
		zoom_en = 2;
	} else if(b) {
	    if(it > 50)
		it = 50;
	    x /= sdl_scale;
	    y /= sdl_scale;
	    if(y>=YRES && y<YRES+40) {
		if(!lb) {
		    tx = x/32;
		    ty = (y-YRES)/20;
		    c = 2*tx+ty;
		    if(c>=PT_NUM) {
			tx = (x+512-XRES)/32;
			c = 2*tx+ty;
		    }
		    if((c>=0 && c<PT_NUM) || (c>=22 && c<32)) {
			if(b&1) {
			    sl = c;
			    su = c;
			} else {
			    sr = c;
			    su = c;
			}
		    }
		    lb = 0;
		}
	    } else if(y>=YRES+40) {
		if(!lb) {
		    if(x>=189 && x<=365 && svf_login && svf_open)
			tag_list_ui(vid_buf);
		    if(x>=367 && x<=383 && !bq) {
			memset(signs, 0, sizeof(signs));
			memset(pv, 0, sizeof(pv));
			memset(vx, 0, sizeof(vx));
			memset(vy, 0, sizeof(vy));
			memset(fvx, 0, sizeof(fvx));
			memset(fvy, 0, sizeof(fvy));
			memset(bmap, 0, sizeof(bmap));
			memset(emap, 0, sizeof(emap));
			memset(parts, 0, sizeof(particle)*NPART);
			for(i=0; i<NPART-1; i++)
			    parts[i].life = i+1;
			parts[NPART-1].life = -1;
			pfree = 0;

			svf_open = 0;
			svf_own = 0;
			svf_id[0] = 0;
			svf_name[0] = 0;
			svf_tags[0] = 0;

			memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
			memset(fire_r, 0, sizeof(fire_r));
			memset(fire_g, 0, sizeof(fire_g));
			memset(fire_b, 0, sizeof(fire_b));
		    }
		    if(x>=385 && x<=476) {
			login_ui(vid_buf);
			if(svf_login)
			    save_presets(0);
		    }
		    if(x>=37 && x<=187 && svf_login) {
			if(!svf_open || !svf_own || x>51) {
			    if(save_name_ui(vid_buf))
				execute_save(vid_buf);
			} else
			    execute_save(vid_buf);
			while(!sdl_poll())
			    if(!SDL_GetMouseState(&x, &y))
				break;
			b = bq = 0;
		    }
		    if(x>=1 && x<=17) {
			search_ui(vid_buf);
			memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
			memset(fire_r, 0, sizeof(fire_r));
			memset(fire_g, 0, sizeof(fire_g));
			memset(fire_b, 0, sizeof(fire_b));
		    }
		    if(x>=19 && x<=35 && svf_last && svf_open)
			parse_save(svf_last, svf_lsize, 1, 0, 0);
		    if(x>=476 && x<=491 && !bq) {
			if(b & SDL_BUTTON_LMASK)
			    set_cmode((cmode+1) % 4);
			if(b & SDL_BUTTON_RMASK)
			    set_cmode((cmode+3) % 4);
			save_presets(0);
		    }
		    if(x>=494 && x<=509 && !bq)
			sys_pause = !sys_pause;
		    lb = 0;
		}
	    } else {
		c = (b&1) ? sl : sr;
		su = c;
		if(c==26) {
		    if(!bq)
			add_sign_ui(vid_buf, x, y);
		} else if(lb) {
		    if(lm == 1) {
			xor_line(lx, ly, x, y, vid_buf);
			if(c==27 && lx>=0 && ly>=0 && lx<XRES && ly<YRES && bmap[ly/CELL][lx/CELL]==4) {
			    nfvx = (x-lx)*0.005f;
			    nfvy = (y-ly)*0.005f;
			    flood_parts(lx, ly, 255, -1, 4);
			    for(j=0; j<YRES/CELL; j++)
				for(i=0; i<XRES/CELL; i++)
				    if(bmap[j][i] == 255) {
					fvx[j][i] = nfvx;
					fvy[j][i] = nfvy;
					bmap[j][i] = 4;
				    }
			}
		    } else if(lm == 2) {
			xor_line(lx, ly, lx, y, vid_buf);
			xor_line(lx, y, x, y, vid_buf);
			xor_line(x, y, x, ly, vid_buf);
			xor_line(x, ly, lx, ly, vid_buf);
		    } else {
			create_line(lx, ly, x, y, bs, c);
			lx = x;
			ly = y;
		    }
		} else {
		    if((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT)) && !(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))) {
			lx = x;
			ly = y;
			lb = b;
			lm = 1;
		    } else if((sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)) && !(sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))) {
			lx = x;
			ly = y;
			lb = b;
			lm = 2;
		    } else if((sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)) && (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT))) {
			if(c!=25)
			    flood_parts(x, y, c, -1, -1);
			lx = x;
			ly = y;
			lb = 0;
			lm = 0;
		    } else {
		        create_parts(x, y, bs, c);
			lx = x;
			ly = y;
			lb = b;
			lm = 0;
		    }
		}
	    }
	} else {
	    if(lb && lm) {
		x /= sdl_scale;
		y /= sdl_scale;
		c = (lb&1) ? sl : sr;
		su = c;
		if(lm == 1) {
		    if(c!=27 || lx<0 || ly<0 || lx>=XRES || ly>=YRES || bmap[ly/CELL][lx/CELL]!=4)
			create_line(lx, ly, x, y, bs, c);
		} else
		    create_box(lx, ly, x, y, c);
		lm = 0;
	    }
	    lb = 0;
	}

	if(load_mode) {
	    draw_image(vid_buf, load_img, load_x, load_y, load_w, load_h, 128);
	    xor_rect(vid_buf, load_x, load_y, load_w, load_h);
	}

	if(save_mode) {
	    xor_rect(vid_buf, save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
	    da = 51;
	    db = 269;
	}

	if(zoom_en!=1 && !load_mode && !save_mode)
	    render_cursor(vid_buf, mx/sdl_scale, my/sdl_scale, su, bs);

	if(zoom_en)
	    render_zoom(vid_buf);

	if(da)
	    switch(db) {
	    case 22:
		drawtext(vid_buf, 16, YRES-24, "Wall. Indestructible. Blocks everything. Conductive.", 255, 255, 255, da*5);
		break;
	    case 23:
		drawtext(vid_buf, 16, YRES-24, "E-Wall. Becomes transparent when electricity is connected.", 255, 255, 255, da*5);
		break;
	    case 24:
		drawtext(vid_buf, 16, YRES-24, "Detector. Generates electricity when a particle is inside.", 255, 255, 255, da*5);
		break;
	    case 25:
		drawtext(vid_buf, 16, YRES-24, "Streamline. Set start point of a streamline.", 255, 255, 255, da*5);
		break;
	    case 26:
		drawtext(vid_buf, 16, YRES-24, "Sign. Click on a sign to edit it or anywhere else to place a new one.", 255, 255, 255, da*5);
		break;
	    case 27:
		drawtext(vid_buf, 16, YRES-24, "Fan. Accelerates air. Use line tool to set direction and strength.", 255, 255, 255, da*5);
		break;
	    case 28:
		drawtext(vid_buf, 16, YRES-24, "Wall. Blocks most particles but lets liquids through. Conductive.", 255, 255, 255, da*5);
		break;
	    case 29:
		drawtext(vid_buf, 16, YRES-24, "Wall. Absorbs particles but lets air currents through.", 255, 255, 255, da*5);
		break;
	    case 30:
		drawtext(vid_buf, 16, YRES-24, "Erases walls.", 255, 255, 255, da*5);
		break;
	    case 31:
		drawtext(vid_buf, 16, YRES-24, "Wall. Indestructible. Blocks everything.", 255, 255, 255, da*5);
		break;
	    case 256:
		drawtext(vid_buf, 16, YRES-24, "Add simulation tags.", 255, 255, 255, da*5);
		break;
	    case 257:
		drawtext(vid_buf, 16, YRES-24, "Add and remove simulation tags.", 255, 255, 255, da*5);
		break;
	    case 258:
		drawtext(vid_buf, 16, YRES-24, "Save the simulation under the current name.", 255, 255, 255, da*5);
		break;
	    case 259:
		drawtext(vid_buf, 16, YRES-24, "Save the simulation under a new name.", 255, 255, 255, da*5);
		break;
	    case 260:
		drawtext(vid_buf, 16, YRES-24, "Sign into the Simulation Server.", 255, 255, 255, da*5);
		break;
	    case 261:
		drawtext(vid_buf, 16, YRES-24, "Sign into the Simulation Server under a new name.", 255, 255, 255, da*5);
		break;
	    case 262:
		drawtext(vid_buf, 16, YRES-24, "Find & open a simulation", 255, 255, 255, da*5);
		break;
	    case 263:
		drawtext(vid_buf, 16, YRES-24, "Pause the simulation", 255, 255, 255, da*5);
		break;
	    case 264:
		drawtext(vid_buf, 16, YRES-24, "Resume the simulation", 255, 255, 255, da*5);
		break;
	    case 265:
		drawtext(vid_buf, 16, YRES-24, "Reload the simulation", 255, 255, 255, da*5);
		break;
	    case 266:
		drawtext(vid_buf, 16, YRES-24, "Erase all particles and walls", 255, 255, 255, da*5);
		break;
	    case 267:
		drawtext(vid_buf, 16, YRES-24, "Change display mode", 255, 255, 255, da*5);
		break;
	    case 268:
		drawtext(vid_buf, 16, YRES-24, "Annuit C\245ptis", 255, 255, 255, da*5);
		break;
	    case 269:
		drawtext(vid_buf, 16, YRES-24, "Click-and-drag to specify a rectangle to copy (right click = cancel).", 255, 216, 32, da*5);
		break;
	    default:
		drawtext(vid_buf, 16, YRES-24, descs[db], 255, 255, 255, da*5);
	    }
	if(itc) {
	    itc--;
	    drawtext(vid_buf, (XRES-textwidth(itc_msg))/2, ((YRES/2)-10), itc_msg, 255, 255, 255, itc>51?255:itc*5);
	}
	if(it) {
	    it--;
	    if(fbi_img)
		draw_image(vid_buf, fbi_img, (XRES-FBI_W)/2, (YRES-FBI_H)/2+25, FBI_W, FBI_H, it>12?64:it*5);
	    drawtext(vid_buf, 16, 20, it_msg, 255, 255, 255, it>51?255:it*5);
	}

	if(old_version) {
	    clearrect(vid_buf, 512-21-old_ver_len, YRES-24, old_ver_len+9, 17);
	    drawtext(vid_buf, 512-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
	    drawrect(vid_buf, 512-19-old_ver_len, YRES-22, old_ver_len+5, 13, 255, 216, 32, 255);
	}

	sdl_blit(0, 0, XRES, YRES+60, vid_buf, XRES);
    }

    http_done();
    return 0;
}
