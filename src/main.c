/**
 * Powder Toy - Main source
 *
 * Copyright (c) 2008 - 2010 Stanislaw Skowronek.
 * Copyright (c) 2010 Simon Robertshaw
 * Copyright (c) 2010 Skresanov Savely
 * Copyright (c) 2010 Bryan Hoyle
 * Copyright (c) 2010 Nathan Cousins
 * Copyright (c) 2010 cracker64
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
#include <SDL/SDL_audio.h>
#include <bzlib.h>
#include <time.h>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <misc.h>
#include <font.h>
#include <defines.h>
#include <powder.h>
#include <graphics.h>
#include <version.h>
#include <http.h>
#include <md5.h>
#include <update.h>
#include <hmap.h>
#include <air.h>
#include <icon.h>

#define NUM_SOUNDS 2
struct sample {
    Uint8 *data;
    Uint32 dpos;
    Uint32 dlen;
} sounds[NUM_SOUNDS];

void mixaudio(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint32 amount;

    for ( i=0; i<NUM_SOUNDS; ++i ) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
        sounds[i].dpos += amount;
    }
}

void play_sound(char *file)
{
    int index;
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    /* Look for an empty (or finished) sound slot */
    for ( index=0; index<NUM_SOUNDS; ++index ) {
        if ( sounds[index].dpos == sounds[index].dlen ) {
            break;
        }
    }
    if ( index == NUM_SOUNDS )
        return;

    /* Load the sound file and convert it to 16-bit stereo at 22kHz */
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
                            AUDIO_S16,   2,             22050);
    cvt.buf = malloc(dlen*cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);

    /* Put the sound data in the slot (it starts playing immediately) */
    if ( sounds[index].data ) {
        free(sounds[index].data);
    }
    SDL_LockAudio();
    sounds[index].data = cvt.buf;
    sounds[index].dlen = cvt.len_cvt;
    sounds[index].dpos = 0;
    SDL_UnlockAudio();
}

static const char *it_msg =
    "\brThe Powder Toy - http://powdertoy.co.uk/\n"
    "\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\x7F\n"
    "\n"
    "\bgControl+C/V/X are Copy, Paste and cut respectively.\n"
    "\bgTo choose a material, hover over one of the icons on the right, it will show a selection of elements in that group.\n"
    "\bgPick your material from the menu using mouse left/right buttons.\n"
    "Draw freeform lines by dragging your mouse left/right button across the drawing area.\n"
    "Shift+drag will create straight lines of particles.\n"
    "Ctrl+drag will result in filled rectangles.\n"
    "Ctrl+Shift+click will flood-fill a closed area.\n"
    "Ctrl+Z will act as Undo.\n"
    "Middle click or Alt+Click to \"sample\" the particles.\n"
    "\n\boUse 'Z' for a zoom tool. Click to make the drawable zoom window stay around. Use the wheel to change the zoom strength\n"
    "Use 'S' to save parts of the window as 'stamps'.\n"
    "'L' will load the most recent stamp, 'K' shows a library of stamps you saved.\n"
    "'C' will cycle the display mode (Fire, Blob, Velocity, etc.). The numbers on the keyboard do the same\n"
    "Use the mouse scroll wheel to change the tool size for particles.\n"
    "The spacebar can be used to pause physics.\n"
    "'P' will take a screenshot and save it into the current directory.\n"
    "\n"
    "\bgCopyright (c) 2008-10 Stanislaw K Skowronek (\brhttp://powder.unaligned.org\bg, \bbirc.unaligned.org #wtf\bg)\n"
    "\bgCopyright (c) 2010 Simon Robertshaw (\brhttp://powdertoy.co.uk\bg, \bbirc.freenode.net #powder\bg)\n"
    "\bgCopyright (c) 2010 Skresanov Savely (Stickman)\n"
    "\bgCopyright (c) 2010 cracker64\n"
    "\bgCopyright (c) 2010 Bryan Hoyle (New elements)\n"
    "\bgCopyright (c) 2010 Nathan Cousins (New elements, small engine mods.)\n"
    "\n"
    "\bgTo use online features such as saving, you need to register at: \brhttp://powdertoy.co.uk/Register.html"
    ;

typedef struct
{
	int start, inc;
	pixel *vid;
} upstruc;

#ifdef BETA
static const char *old_ver_msg_beta = "A new beta is available - click here!";
#endif
static const char *old_ver_msg = "A new version is available - click here!";
float mheat = 0.0f;

int do_open = 0;
int sys_pause = 0;
int legacy_enable = 0; //Used to disable new features such as heat, will be set by commandline or save.
int death = 0, framerender = 0;
int amd = 1;
int FPSB = 0;
int MSIGN =-1;
//int CGOL = 0;
//int GSPEED = 1;//causes my .exe to crash..

sign signs[MAXSIGNS];

int numCores = 4;

int core_count()
{
	int numCPU = 1;
#ifdef MT
#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	numCPU = sysinfo.dwNumberOfProcessors;
#else
#ifdef MACOSX
	numCPU = 4;
#else
	numCPU = sysconf( _SC_NPROCESSORS_ONLN );
#endif
#endif

	printf("Cpus: %d\n", numCPU);
	if (numCPU>1)
		printf("Multithreading enabled\n");
	else
		printf("Multithreading disabled\n");
#endif
	return numCPU;
}

int mousex = 0, mousey = 0;  //They contain mouse position
int kiosk_enable = 0;

void sdl_seticon(void)
{
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
}

int frame_idx=0;
void dump_frame(pixel *src, int w, int h, int pitch)
{
	char frame_name[32];
	int j,i;
	unsigned char c[3];
	FILE *f;
	sprintf(frame_name,"frame%04d.ppm",frame_idx);
	f=fopen(frame_name,"wb");
	fprintf(f,"P6\n%d %d\n255\n",w,h);
	for (j=0; j<h; j++)
	{
		for (i=0; i<w; i++)
		{
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

/***********************************************************
 *                    STATE MANAGEMENT                     *
 ***********************************************************/

void *build_thumb(int *size, int bzip2)
{
	unsigned char *d=calloc(1,XRES*YRES), *c;
	int i,j,x,y;
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=0 && x<XRES && y>=0 && y<YRES)
				d[x+y*XRES] = parts[i].type;
		}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
			if (bmap[y][x])
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						d[x*CELL+i+(y*CELL+j)*XRES] = 0xFF;
	j = XRES*YRES;

	if (bzip2)
	{
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

		if (BZ2_bzBuffToBuffCompress((char *)(c+8), (unsigned *)&i, (char *)d, j, 9, 0, 0) != BZ_OK)
		{
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

void *build_save(int *size, int x0, int y0, int w, int h)
{
	unsigned char *d=calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*11+MAXSIGNS*262), *c;
	int i,j,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int));
	int bx0=x0/CELL, by0=y0/CELL, bw=(w+CELL-1)/CELL, bh=(h+CELL-1)/CELL;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	// save the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			d[p++] = bmap[y][x];
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (bmap[y][x]==WL_FAN||bmap[y][x]==4)
			{
				i = (int)(fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (bmap[y][x]==WL_FAN||bmap[y][x]==4)
			{
				i = (int)(fvy[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}

	// save the particle map
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=x0 && x<x0+w && y>=y0 && y<y0+h) {
				if (!m[(x-x0)+(y-y0)*w] ||
				        parts[m[(x-x0)+(y-y0)*w]-1].type == PT_PHOT)
					m[(x-x0)+(y-y0)*w] = i+1;
			}
		}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
			d[p++] = parts[i-1].type;
		else
			d[p++] = 0;
	}

	// save particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			x = (int)(parts[i].vx*16.0f+127.5f);
			y = (int)(parts[i].vy*16.0f+127.5f);
			if (x<0) x=0;
			if (x>255) x=255;
			if (y<0) y=0;
			if (y>255) y=255;
			d[p++] = x;
			d[p++] = y;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Everybody loves a 16bit int
			//d[p++] = (parts[i-1].life+3)/4;
			int ttlife = (int)parts[i-1].life;
			d[p++] = ((ttlife&0xFF00)>>8);
			d[p++] = (ttlife&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Now saving tmp!
			//d[p++] = (parts[i-1].life+3)/4;
			int tttmp = (int)parts[i-1].tmp;
			d[p++] = ((tttmp&0xFF00)>>8);
			d[p++] = (tttmp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			//New Temperature saving uses a 16bit unsigned int for temperatures, giving a precision of 1 degree versus 36 for the old format
			int tttemp = (int)parts[i-1].temp;
			d[p++] = ((tttemp&0xFF00)>>8);
			d[p++] = (tttemp&0x00FF);
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i && (parts[i-1].type==PT_CLNE || parts[i-1].type==PT_PCLN || parts[i-1].type==PT_BCLN || parts[i-1].type==PT_SPRK || parts[i-1].type==PT_LAVA || parts[i-1].type==PT_PIPE))
			d[p++] = parts[i-1].ctype;
	}

	j = 0;
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0] &&
		        signs[i].x>=x0 && signs[i].x<x0+w &&
		        signs[i].y>=y0 && signs[i].y<y0+h)
			j++;
	d[p++] = j;
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0] &&
		        signs[i].x>=x0 && signs[i].x<x0+w &&
		        signs[i].y>=y0 && signs[i].y<y0+h)
		{
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

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	c[0] = 0x50;	//0x66;
	c[1] = 0x53;	//0x75;
	c[2] = 0x76;	//0x43;
	c[3] = legacy_enable|((sys_pause<<1)&0x02);
	c[4] = SAVE_VERSION;
	c[5] = CELL;
	c[6] = bw;
	c[7] = bh;
	c[8] = p;
	c[9] = p >> 8;
	c[10] = p >> 16;
	c[11] = p >> 24;

	i -= 12;

	if (BZ2_bzBuffToBuffCompress((char *)(c+12), (unsigned *)&i, (char *)d, p, 9, 0, 0) != BZ_OK)
	{
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
	int q,i,j,k,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int)), ver, pty, ty, legacy_beta=0;
	int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
	int fp[NPART], nf=0, new_format = 0, ttv = 0;

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (size<16)
		return 1;
	if (!(c[2]==0x43 && c[1]==0x75 && c[0]==0x66) && !(c[2]==0x76 && c[1]==0x53 && c[0]==0x50))
		return 1;
	if (c[2]==0x76 && c[1]==0x53 && c[0]==0x50) {
		new_format = 1;
	}
	if (c[4]>SAVE_VERSION)
		return 2;
	ver = c[4];

	if (ver<34)
	{
		legacy_enable = 1;
	}
	else
	{
		if (ver>=44) {
			legacy_enable = c[3]&0x01;
			if (!sys_pause) {
				sys_pause = (c[3]>>1)&0x01;
			}
		} else {
			if (c[3]==1||c[3]==0) {
				legacy_enable = c[3];
			} else {
				legacy_beta = 1;
			}
		}
	}

	bw = c[6];
	bh = c[7];
	if (bx0+bw > XRES/CELL)
		bx0 = XRES/CELL - bw;
	if (by0+bh > YRES/CELL)
		by0 = YRES/CELL - bh;
	if (bx0 < 0)
		bx0 = 0;
	if (by0 < 0)
		by0 = 0;

	if (c[5]!=CELL || bx0+bw>XRES/CELL || by0+bh>YRES/CELL)
		return 3;
	i = (unsigned)c[8];
	i |= ((unsigned)c[9])<<8;
	i |= ((unsigned)c[10])<<16;
	i |= ((unsigned)c[11])<<24;
	d = malloc(i);
	if (!d)
		return 1;

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), size-12, 0, 0))
		return 1;
	size = i;

	if (size < bw*bh)
		return 1;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	if (replace)
	{
		gravityMode = 1;

		memset(bmap, 0, sizeof(bmap));
		memset(emap, 0, sizeof(emap));
		memset(signs, 0, sizeof(signs));
		memset(parts, 0, sizeof(particle)*NPART);
		memset(pmap, 0, sizeof(pmap));
		memset(vx, 0, sizeof(vx));
		memset(vy, 0, sizeof(vy));
		memset(pv, 0, sizeof(pv));
		memset(photons, 0, sizeof(photons));
		memset(wireless, 0, sizeof(wireless));
		memset(gol2, 0, sizeof(gol2));
		memset(portal, 0, sizeof(portal));
		death = death2 = ISSPAWN1 = ISSPAWN2 = 0;
	}

	// make a catalog of free parts
	memset(pmap, 0, sizeof(pmap));
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			pmap[y][x] = (i<<8)|1;
		}
		else
			fp[nf++] = i;

	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				bmap[y][x] = d[p];
				if (bmap[y][x]==1)
					bmap[y][x]=WL_WALL;
				if (bmap[y][x]==2)
					bmap[y][x]=WL_DESTROYALL;
				if (bmap[y][x]==3)
					bmap[y][x]=WL_ALLOWLIQUID;
				if (bmap[y][x]==4)
					bmap[y][x]=WL_FAN;
				if (bmap[y][x]==5)
					bmap[y][x]=WL_STREAM;
				if (bmap[y][x]==6)
					bmap[y][x]=WL_DETECT;
				if (bmap[y][x]==7)
					bmap[y][x]=WL_EWALL;
				if (bmap[y][x]==8)
					bmap[y][x]=WL_WALLELEC;
				if (bmap[y][x]==9)
					bmap[y][x]=WL_ALLOWAIR;
				if (bmap[y][x]==10)
					bmap[y][x]=WL_ALLOWSOLID;
				if (bmap[y][x]==11)
					bmap[y][x]=WL_ALLOWALLELEC;
				if (bmap[y][x]==12)
					bmap[y][x]=WL_EHOLE;
				if (bmap[y][x]==13)
					bmap[y][x]=WL_ALLOWGAS;
			}

			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= size)
					goto corrupt;
				fvx[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= size)
					goto corrupt;
				fvy[y][x] = (d[p++]-127.0f)/64.0f;
			}

	// load the particle map
	i = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= size)
				goto corrupt;
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//goto corrupt;
			}
			gol[x][y]=0;
			if (j)// && !(isplayer == 1 && j==PT_STKM))
			{
				if (pmap[y][x])
				{
					k = pmap[y][x]>>8;
					parts[k].type = j;
					if (j == PT_PHOT)
						parts[k].ctype = 0x3fffffff;
					parts[k].x = (float)x;
					parts[k].y = (float)y;
					m[(x-x0)+(y-y0)*w] = k+1;
				}
				else if (i < nf)
				{
					parts[fp[i]].type = j;
					if (j == PT_COAL)
						parts[fp[i]].tmp = 50;
					if (j == PT_FUSE)
						parts[fp[i]].tmp = 50;
					if (j == PT_PHOT)
						parts[fp[i]].ctype = 0x3fffffff;
					parts[fp[i]].x = (float)x;
					parts[fp[i]].y = (float)y;
					m[(x-x0)+(y-y0)*w] = fp[i]+1;
					i++;
				}
				else
					m[(x-x0)+(y-y0)*w] = NPART+1;
			}
		}

	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= size)
				goto corrupt;
			if (i < NPART)
			{
				parts[i].vx = (d[p++]-127.0f)/16.0f;
				parts[i].vy = (d[p++]-127.0f)/16.0f;
				if (parts[i].type == PT_STKM)
				{
					//player[2] = PT_DUST;

					player[3] = parts[i].x-1;  //Setting legs positions
					player[4] = parts[i].y+6;
					player[5] = parts[i].x-1;
					player[6] = parts[i].y+6;

					player[7] = parts[i].x-3;
					player[8] = parts[i].y+12;
					player[9] = parts[i].x-3;
					player[10] = parts[i].y+12;

					player[11] = parts[i].x+1;
					player[12] = parts[i].y+6;
					player[13] = parts[i].x+1;
					player[14] = parts[i].y+6;

					player[15] = parts[i].x+3;
					player[16] = parts[i].y+12;
					player[17] = parts[i].x+3;
					player[18] = parts[i].y+12;

				}
				if (parts[i].type == PT_STKM2)
				{
					//player[2] = PT_DUST;

					player2[3] = parts[i].x-1;  //Setting legs positions
					player2[4] = parts[i].y+6;
					player2[5] = parts[i].x-1;
					player2[6] = parts[i].y+6;

					player2[7] = parts[i].x-3;
					player2[8] = parts[i].y+12;
					player2[9] = parts[i].x-3;
					player2[10] = parts[i].y+12;

					player2[11] = parts[i].x+1;
					player2[12] = parts[i].y+6;
					player2[13] = parts[i].x+1;
					player2[14] = parts[i].y+6;

					player2[15] = parts[i].x+3;
					player2[16] = parts[i].y+12;
					player2[17] = parts[i].x+3;
					player2[18] = parts[i].y+12;

				}
			}
			else
				p += 2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=44) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].life = ttv;
				} else {
					p+=2;
				}
			} else {
				if (p >= size)
					goto corrupt;
				if (i <= NPART)
					parts[i-1].life = d[p++]*4;
				else
					p++;
			}
		}
	}
	if (ver>=44) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			if (i)
			{
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].tmp = ttv;
					if(ptypes[parts[i-1].type].properties&PROP_LIFE && !parts[i-1].tmp)
						for(q = 1; q<=NGOL ; q++) {
							if(parts[i-1].type==goltype[q-1] && grule[q][9]==2)
								parts[i-1].tmp = grule[q][9]-1;
						}
				} else {
					p+=2;
				}
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
		if (i)
		{
			if (ver>=34&&legacy_beta==0)
			{
				if (p >= size)
				{
					goto corrupt;
				}
				if (i <= NPART)
				{
					if (ver>=42) {
						if (new_format) {
							ttv = (d[p++])<<8;
							ttv |= (d[p++]);
							if (parts[i-1].type==PT_PUMP) {
								parts[i-1].temp = ttv + 0.15;//fix PUMP saved at 0, so that it loads at 0.
							} else {
								parts[i-1].temp = ttv;
							}
						} else {
							parts[i-1].temp = (d[p++]*((MAX_TEMP+(-MIN_TEMP))/255))+MIN_TEMP;
						}
					} else {
						parts[i-1].temp = ((d[p++]*((O_MAX_TEMP+(-O_MIN_TEMP))/255))+O_MIN_TEMP)+273;
					}
				}
				else
				{
					p++;
					if (new_format) {
						p++;
					}
				}
			}
			else
			{
				parts[i-1].temp = ptypes[parts[i-1].type].heat;
			}
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43)))
		{
			if (p >= size)
				goto corrupt;
			if (i <= NPART)
				parts[i-1].ctype = d[p++];
			else
				p++;
		}
	}

	if (p >= size)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > size)
			goto corrupt;
		for (k=0; k<MAXSIGNS; k++)
			if (!signs[k].text[0])
				break;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			signs[k].x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			signs[k].y = x+y0;
		x = d[p++];
		if (k<MAXSIGNS)
			signs[k].ju = x;
		x = d[p++];
		if (p+x > size)
			goto corrupt;
		if (k<MAXSIGNS)
		{
			memcpy(signs[k].text, d+p, x);
			signs[k].text[x] = 0;
		}
		p += x;
	}

version1:
	free(d);

	return 0;

corrupt:
	if (replace)
	{
		legacy_enable = 0;
		memset(signs, 0, sizeof(signs));
		memset(parts, 0, sizeof(particle)*NPART);
		memset(bmap, 0, sizeof(bmap));
	}
	return 1;
}

// stamps library

stamp stamps[STAMP_MAX];//[STAMP_X*STAMP_Y];

int stamp_count = 0;

unsigned last_time=0, last_name=0;
void stamp_gen_name(char *fn)
{
	unsigned t=(unsigned)time(NULL);

	if (last_time!=t)
	{
		last_time=t;
		last_name=0;
	}
	else
		last_name++;

	sprintf(fn, "%08x%02x", last_time, last_name);
}

void stamp_update(void)
{
	FILE *f;
	int i;
	f=fopen("stamps" PATH_SEP "stamps.def", "wb");
	if (!f)
		return;
	for (i=0; i<STAMP_MAX; i++)
	{
		if (!stamps[i].name[0])
			break;
		if (stamps[i].dodelete!=1)
		{
			fwrite(stamps[i].name, 1, 10, f);
		}
	}
	fclose(f);
}

void stamp_gen_thumb(int i)
{
	char fn[64];
	void *data;
	int size, factor_x, factor_y;
	pixel *tmp;

	if (stamps[i].thumb)
	{
		free(stamps[i].thumb);
		stamps[i].thumb = NULL;
	}

	sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
	data = file_load(fn, &size);

	if (data)
	{
		stamps[i].thumb = prerender_save(data, size, &(stamps[i].thumb_w), &(stamps[i].thumb_h));
		if (stamps[i].thumb && (stamps[i].thumb_w>XRES/GRID_S || stamps[i].thumb_h>YRES/GRID_S))
		{
			factor_x = ceil((float)stamps[i].thumb_w/(float)(XRES/GRID_S));
			factor_y = ceil((float)stamps[i].thumb_h/(float)(YRES/GRID_S));
			if (factor_y > factor_x)
				factor_x = factor_y;
			tmp = rescale_img(stamps[i].thumb, stamps[i].thumb_w, stamps[i].thumb_h, &(stamps[i].thumb_w), &(stamps[i].thumb_h), factor_x);
			free(stamps[i].thumb);
			stamps[i].thumb = tmp;
		}
	}

	free(data);
}

int clipboard_ready = 0;
void *clipboard_data = 0;
int clipboard_length = 0;

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
	if (!f)
		return;
	fwrite(s, n, 1, f);
	fclose(f);

	free(s);

	if (stamps[STAMP_MAX-1].thumb)
		free(stamps[STAMP_MAX-1].thumb);
	memmove(stamps+1, stamps, sizeof(struct stamp)*(STAMP_MAX-1));
	memset(stamps, 0, sizeof(struct stamp));
	if (stamp_count<STAMP_MAX)
		stamp_count++;

	strcpy(stamps[0].name, sn);
	stamp_gen_thumb(0);

	stamp_update();
}

void *stamp_load(int i, int *size)
{
	void *data;
	char fn[64];
	struct stamp tmp;

	if (!stamps[i].thumb || !stamps[i].name[0])
		return NULL;

	sprintf(fn, "stamps" PATH_SEP "%s.stm", stamps[i].name);
	data = file_load(fn, size);
	if (!data)
		return NULL;

	if (i>0)
	{
		memcpy(&tmp, stamps+i, sizeof(struct stamp));
		memmove(stamps+1, stamps, sizeof(struct stamp)*i);
		memcpy(stamps, &tmp, sizeof(struct stamp));

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
	if (!f)
		return;
	for (i=0; i<STAMP_MAX; i++)
	{
		fread(stamps[i].name, 1, 10, f);
		if (!stamps[i].name[0])
			break;
		stamp_count++;
		stamp_gen_thumb(i);
	}
	fclose(f);
}

void del_stamp(int d)
{
	stamps[d].dodelete = 1;
	stamp_update();
	stamp_count = 0;
	stamp_init();
}

void thumb_cache_inval(char *id);

char *thumb_cache_id[THUMB_CACHE_SIZE];
void *thumb_cache_data[THUMB_CACHE_SIZE];
int thumb_cache_size[THUMB_CACHE_SIZE];
int thumb_cache_lru[THUMB_CACHE_SIZE];

void thumb_cache_inval(char *id)
{
	int i,j;
	for (i=0; i<THUMB_CACHE_SIZE; i++)
		if (thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
			break;
	if (i >= THUMB_CACHE_SIZE)
		return;
	free(thumb_cache_id[i]);
	free(thumb_cache_data[i]);
	thumb_cache_id[i] = NULL;
	for (j=0; j<THUMB_CACHE_SIZE; j++)
		if (thumb_cache_lru[j] > thumb_cache_lru[i])
			thumb_cache_lru[j]--;
}
void thumb_cache_add(char *id, void *thumb, int size)
{
	int i,m=-1,j=-1;
	thumb_cache_inval(id);
	for (i=0; i<THUMB_CACHE_SIZE; i++)
	{
		if (!thumb_cache_id[i])
			break;
		if (thumb_cache_lru[i] > m)
		{
			m = thumb_cache_lru[i];
			j = i;
		}
	}
	if (i >= THUMB_CACHE_SIZE)
	{
		thumb_cache_inval(thumb_cache_id[j]);
		i = j;
	}
	for (j=0; j<THUMB_CACHE_SIZE; j++)
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
	for (i=0; i<THUMB_CACHE_SIZE; i++)
		if (thumb_cache_id[i] && !strcmp(id, thumb_cache_id[i]))
			break;
	if (i >= THUMB_CACHE_SIZE)
		return 0;
	for (j=0; j<THUMB_CACHE_SIZE; j++)
		if (thumb_cache_lru[j] < thumb_cache_lru[i])
			thumb_cache_lru[j]++;
	thumb_cache_lru[i] = 0;
	*thumb = malloc(thumb_cache_size[i]);
	*size = thumb_cache_size[i];
	memcpy(*thumb, thumb_cache_data[i], *size);
	return 1;
}

char http_proxy_string[256] = "";

unsigned char last_major=0, last_minor=0, update_flag=0;

char *tag = "(c) 2008-9 Stanislaw Skowronek";
int itc = 0;
char itc_msg[64] = "[?]";

char my_uri[] = "http://" SERVER "/Update.api?Action=Download&Architecture="
#if defined WIN32
                "Windows32"
#elif defined LIN32
                "Linux32"
#elif defined LIN64
                "Linux64"
#elif defined MACOSX
                "MacOSX"
#else
                "Unknown"
#endif
                "&InstructionSet="
#if defined X86_SSE3
                "SSE3"
#elif defined X86_SSE2
                "SSE2"
#elif defined X86_SSE
                "SSE"
#else
                "SSE"
#endif
                ;
int main(int argc, char *argv[])
{
	int hud_enable = 1;
	int active_menu = 0;
#ifdef BETA
	int is_beta = 0;
#endif
	char uitext[255] = "";
	char heattext[128] = "";
	char coordtext[13] = "";
	int currentTime = 0;
	int FPS = 0;
	int pastFPS = 0;
	int past = 0;
	pixel *vid_buf=calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	pixel *pers_bg=calloc((XRES+BARSIZE)*YRES, PIXELSIZE);
	void *http_ver_check;
	void *http_session_check = NULL;
	char *ver_data=NULL, *check_data=NULL, *tmp;
	char console_error[255] = "";
	int i, j, bq, fire_fc=0, do_check=0, do_s_check=0, old_version=0, http_ret=0,http_s_ret=0, major, minor, old_ver_len;
#ifdef INTERNAL
	int vs = 0;
#endif
	int x, y, b = 0, sl=1, sr=0, su=0, c, lb = 0, lx = 0, ly = 0, lm = 0;//, tx, ty;
	int da = 0, db = 0, it = 2047, mx, my, bsx = 2, bsy = 2;
	float nfvx, nfvy;
	int load_mode=0, load_w=0, load_h=0, load_x=0, load_y=0, load_size=0;
	void *load_data=NULL;
	pixel *load_img=NULL;//, *fbi_img=NULL;
	int save_mode=0, save_x=0, save_y=0, save_w=0, save_h=0, copy_mode=0;
	GSPEED = 1;
	
	SDL_AudioSpec fmt;
	/* Set 16-bit stereo audio at 22Khz */
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 512;
	fmt.callback = mixaudio;
	fmt.userdata = NULL;
	/* Open the audio device and start playing sound! */
	if ( SDL_OpenAudio(&fmt, NULL) < 0 )
	{
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_PauseAudio(0);
#ifdef MT
	numCores = core_count();
#endif
//TODO: Move out version stuff
#ifdef BETA
	if (is_beta)
	{
		old_ver_len = textwidth(old_ver_msg_beta);
	}
	else
	{
		old_ver_len = textwidth(old_ver_msg);
	}
#else
	old_ver_len = textwidth(old_ver_msg);
#endif
	menu_count();
	parts = calloc(sizeof(particle), NPART);
	cb_parts = calloc(sizeof(particle), NPART);
	for (i=0; i<NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;
	fire_bg=calloc(XRES*YRES, PIXELSIZE);
	memset(signs, 0, sizeof(signs));

	//fbi_img = render_packed_rgb(fbi, FBI_W, FBI_H, FBI_CMP);

	load_presets();

	for (i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "scale:", 6))
		{
			sdl_scale = (argv[i][6]=='2') ? 2 : 1;
		}
		else if (!strncmp(argv[i], "proxy:", 6))
		{
			memset(http_proxy_string, 0, sizeof(http_proxy_string));
			strncpy(http_proxy_string, argv[i]+6, 255);
		}
		else if (!strncmp(argv[i], "nohud", 5))
		{
			hud_enable = 0;
		}
		else if (!strncmp(argv[i], "kiosk", 5))
		{
			kiosk_enable = 1;
			sdl_scale = 2;
			hud_enable = 0;
		}
	}

	save_presets(0);

	make_kernel();
	prepare_alpha();

	stamp_init();

	sdl_open();
	http_init(http_proxy_string[0] ? http_proxy_string : NULL);

	if (cpu_check())
	{
		error_ui(vid_buf, 0, "Unsupported CPU. Try another version.");
		return 1;
	}

#ifdef BETA
	http_ver_check = http_async_req_start(NULL, "http://" SERVER "/Update.api?Action=CheckVersion", NULL, 0, 0);
#else
	http_ver_check = http_async_req_start(NULL, "http://" SERVER "/Update.api?Action=CheckVersion", NULL, 0, 0);
#endif
	if(svf_login){
		http_session_check = http_async_req_start(NULL, "http://" SERVER "/Login.api?Action=CheckSession", NULL, 0, 0);
		http_auth_headers(http_session_check, svf_user_id, NULL, svf_session_id);
	}

	while (!sdl_poll())
	{
		if (!sys_pause||framerender)
		{
			update_air();
		}
#ifdef OpenGL
		ClearScreen();
#else
		if (cmode==CM_VEL || cmode==CM_PRESS || cmode==CM_CRACK)
		{
			draw_air(vid_buf);
		}
		else if (cmode==CM_PERS)
		{
			memcpy(vid_buf, pers_bg, (XRES+BARSIZE)*YRES*PIXELSIZE);
			memset(vid_buf+((XRES+BARSIZE)*YRES), 0, ((XRES+BARSIZE)*YRES*PIXELSIZE)-((XRES+BARSIZE)*YRES*PIXELSIZE));
		}
		else
		{
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
#endif

		//Can't be too sure...
		if (bsx>1180)
			bsx = 1180;
		if (bsx<0)
			bsx = 0;
		if (bsy>1180)
			bsy = 1180;
		if (bsy<0)
			bsy = 0;

		update_particles(vid_buf);
		draw_parts(vid_buf);

		if (cmode==CM_PERS)
		{
			if (!fire_fc)
			{
				dim_copy_pers(pers_bg, vid_buf);
			}
			else
			{
				memcpy(pers_bg, vid_buf, (XRES+BARSIZE)*YRES*PIXELSIZE);
			}
			fire_fc = (fire_fc+1) % 3;
		}
		if (cmode==CM_FIRE||cmode==CM_BLOB||cmode==CM_FANCY)
			render_fire(vid_buf);

		render_signs(vid_buf);

		memset(vid_buf+((XRES+BARSIZE)*YRES), 0, (PIXELSIZE*(XRES+BARSIZE))*MENUSIZE);
		clearrect(vid_buf, XRES-1, 0, BARSIZE+1, YRES);

		draw_svf_ui(vid_buf);

		if (http_ver_check)
		{
			if (!do_check && http_async_req_status(http_ver_check))
			{
				ver_data = http_async_req_stop(http_ver_check, &http_ret, NULL);
				if (http_ret==200 && ver_data)
				{
#ifdef BETA
					if (sscanf(ver_data, "%d.%d.%d", &major, &minor, &is_beta)==3)
						if (major>SAVE_VERSION || (major==SAVE_VERSION && minor>MINOR_VERSION) || (major==SAVE_VERSION && is_beta == 0))
							old_version = 1;
#else
					if (sscanf(ver_data, "%d.%d", &major, &minor)==2)
						if (major>SAVE_VERSION || (major==SAVE_VERSION && minor>MINOR_VERSION))
							old_version = 1;
#endif
					free(ver_data);
				}
				http_ver_check = NULL;
			}
			do_check = (do_check+1) & 15;
		}
		if(http_session_check)
		{
			if(!do_s_check && http_async_req_status(http_session_check))
			{
				check_data = http_async_req_stop(http_session_check, &http_s_ret, NULL);
				if(http_ret==200 && check_data)
				{
					printf("{%s}\n", check_data);
					if(!strncmp(check_data, "EXPIRED", 7))
					{
						//Session expired
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
					}
					else if(!strncmp(check_data, "BANNED", 6))
					{
						//User banned
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
					}
					else if(!strncmp(check_data, "OK", 2))
					{
						//Session valid
						if(strlen(check_data)>2){
							//User is elevated
							if (!strncmp(check_data+3, "ADMIN", 5))
							{
								svf_admin = 1;
								svf_mod = 0;
							}
							else if (!strncmp(check_data+3, "MOD", 3))
							{
								svf_admin = 0;
								svf_mod = 1;
							}							
						}	
						save_presets(0);
					}
					else
					{
						//No idea, but log the user out anyway
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
					}
					free(check_data);
				}
				http_session_check = NULL;
			}
			do_s_check = (do_s_check+1) & 15;
		}

		if (sdl_key=='q' || sdl_key==SDLK_ESCAPE)
		{
			if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
			{
				break;
			}
		}
		//if(sdl_key=='d' && isplayer)
		//{
		//    death = 1;
		//    //death = !(death);
		//}
		if (sdl_key=='f')
		{
			framerender = 1;
		}
		if ((sdl_key=='l' || sdl_key=='k') && stamps[0].name[0])
		{
			if (load_mode)
			{
				free(load_img);
				free(load_data);
				load_mode = 0;
				load_data = NULL;
				load_img = NULL;
			}
			if (it > 50)
				it = 50;
			if (sdl_key=='k' && stamps[1].name[0])
			{
				j = stamp_ui(vid_buf);
				if (j>=0)
					load_data = stamp_load(j, &load_size);
				else
					load_data = NULL;
			}
			else
				load_data = stamp_load(0, &load_size);
			if (load_data)
			{
				load_img = prerender_save(load_data, load_size, &load_w, &load_h);
				if (load_img)
					load_mode = 1;
				else
					free(load_data);
			}
		}
		if (sdl_key=='s' && (sdl_mod & (KMOD_CTRL)) || (sdl_key=='s' && !isplayer2))
		{
			if (it > 50)
				it = 50;
			save_mode = 1;
		}
		if (sdl_key=='1')
		{
			set_cmode(CM_VEL);
		}
		if (sdl_key=='2')
		{
			set_cmode(CM_PRESS);
		}
		if (sdl_key=='3')
		{
			set_cmode(CM_PERS);
		}
		if (sdl_key=='4')
		{
			set_cmode(CM_FIRE);
		}
		if (sdl_key=='5')
		{
			set_cmode(CM_BLOB);
		}
		if (sdl_key=='6')
		{
			set_cmode(CM_HEAT);
		}
		if (sdl_key=='7')
		{
			set_cmode(CM_FANCY);
		}
		if (sdl_key=='8')
		{
			set_cmode(CM_NOTHING);
		}
		if (sdl_key=='9')
		{
			set_cmode(CM_GRAD);
		}
		if (sdl_key=='0')
		{
			set_cmode(CM_CRACK);
		}
		if (sdl_key=='1'&& (sdl_mod & (KMOD_SHIFT)) && DEBUG_MODE)
		{
			set_cmode(CM_LIFE);
		}
		if (sdl_key==SDLK_TAB)
		{
			CURRENT_BRUSH =(CURRENT_BRUSH + 1)%BRUSH_NUM ;
		}
		if (sdl_key==SDLK_LEFTBRACKET) {
			if (sdl_zoom_trig==1)
			{
				ZSIZE -= 1;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
			}
			else
			{
				if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					bsx -= 1;
					bsy -= 1;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					bsx -= 1;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					bsy -= 1;
				}
				else
				{
					bsx -= ceil((bsx/5)+0.5f);
					bsy -= ceil((bsy/5)+0.5f);
				}
				if (bsx>1180)
					bsx = 1180;
				if (bsy>1180)
					bsy = 1180;
				if (bsx<0)
					bsx = 0;
				if (bsy<0)
					bsy = 0;
			}
		}
		if (sdl_key==SDLK_RIGHTBRACKET) {
			if (sdl_zoom_trig==1)
			{
				ZSIZE += 1;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
			}
			else
			{
				if (sdl_mod & (KMOD_LALT|KMOD_RALT) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					bsx += 1;
					bsy += 1;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					bsx += 1;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					bsy += 1;
				}
				else
				{
					bsx += ceil((bsx/5)+0.5f);
					bsy += ceil((bsy/5)+0.5f);
				}
				if (bsx>1180)
					bsx = 1180;
				if (bsy>1180)
					bsy = 1180;
				if (bsx<0)
					bsx = 0;
				if (bsy<0)
					bsy = 0;
			}
		}
		if (sdl_key=='d'&&(sdl_mod & (KMOD_CTRL)) || (sdl_key=='d' && !isplayer2))
			DEBUG_MODE = !DEBUG_MODE;
		if (sdl_key=='i')
		{
			int nx, ny;
			for (nx = 0; nx<XRES/CELL; nx++)
				for (ny = 0; ny<YRES/CELL; ny++)
				{
					pv[ny][nx] = -pv[ny][nx];
					vx[ny][nx] = -vx[ny][nx];
					vy[ny][nx] = -vy[ny][nx];
				}
		}
		if ((sdl_mod & (KMOD_RCTRL) )&&( sdl_mod & (KMOD_RALT)))
			active_menu = 11;
		if (sdl_key==SDLK_INSERT)// || sdl_key==SDLK_BACKQUOTE)
			REPLACE_MODE = !REPLACE_MODE;
		if (sdl_key==SDLK_BACKQUOTE)
		{
			console_mode = !console_mode;
			hud_enable = !console_mode;
		}
		if (sdl_key=='g')
		{
			if (sdl_mod & (KMOD_SHIFT))
				GRID_MODE = (GRID_MODE+9)%10;
			else
				GRID_MODE = (GRID_MODE+1)%10;
		}
		if (sdl_key=='=')
		{
			int nx, ny;
			if(sdl_mod & (KMOD_CTRL))
			{
				for(i=0; i<NPART; i++)
					if(parts[i].type==PT_SPRK)
					{
						parts[i].type = parts[i].ctype;
						parts[i].life = 0;
					}
			}
			else
			{
				for (nx = 0; nx<XRES/CELL; nx++)
					for (ny = 0; ny<YRES/CELL; ny++)
					{
						pv[ny][nx] = 0;
						vx[ny][nx] = 0;
						vy[ny][nx] = 0;
					}
			}
		}

		if (sdl_key=='w' && (!isplayer2 || (sdl_mod & (KMOD_SHIFT)))) //Gravity, by Moach
		{
			++gravityMode; // cycle gravity mode
			itc = 51;

			switch (gravityMode)
			{
			default:
				gravityMode = 0;
			case 0:
				strcpy(itc_msg, "Gravity: Off");
				break;
			case 1:
				strcpy(itc_msg, "Gravity: Vertical");
				break;
			case 2:
				strcpy(itc_msg, "Gravity: Radial");
				break;

			}
		}

		if (sdl_key=='t')
			VINE_MODE = !VINE_MODE;
		if (sdl_key==SDLK_SPACE)
			sys_pause = !sys_pause;
		if (sdl_key=='h')
			hud_enable = !hud_enable;
		if (sdl_key=='p')
			dump_frame(vid_buf, XRES, YRES, XRES+BARSIZE);
		if (sdl_key=='v'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
		{
			if (clipboard_ready==1)
			{
				load_data = malloc(clipboard_length);
				memcpy(load_data, clipboard_data, clipboard_length);
				load_size = clipboard_length;
				if (load_data)
				{
					load_img = prerender_save(load_data, load_size, &load_w, &load_h);
					if (load_img)
						load_mode = 1;
					else
						free(load_data);
				}
			}
		}
		if (sdl_key=='r'&&(sdl_mod & (KMOD_CTRL))&&(sdl_mod & (KMOD_SHIFT)))
		{
			save_mode = 1;
			copy_mode = 4;//invert
		}
		else if (sdl_key=='r'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
		{
			save_mode = 1;
			copy_mode = 3;//rotate
		}
		else if (sdl_key=='r')
			GENERATION = 0;
		if (sdl_key=='x'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
		{
			save_mode = 1;
			copy_mode = 2;
		}
		if (sdl_key=='c'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
		{
			save_mode = 1;
			copy_mode = 1;
		}
		else if (sdl_key=='c')
		{
			set_cmode((cmode+1) % CM_COUNT);
			if (it > 50)
				it = 50;
		}
		if (sdl_key=='z'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))) // Undo
		{
			int cbx, cby, cbi;

			for (cbi=0; cbi<NPART; cbi++)
				parts[cbi] = cb_parts[cbi];

			for (cby = 0; cby<YRES; cby++)
				for (cbx = 0; cbx<XRES; cbx++)
					pmap[cby][cbx] = cb_pmap[cby][cbx];

			for (cby = 0; cby<(YRES/CELL); cby++)
				for (cbx = 0; cbx<(XRES/CELL); cbx++)
				{
					vx[cby][cbx] = cb_vx[cby][cbx];
					vy[cby][cbx] = cb_vy[cby][cbx];
					pv[cby][cbx] = cb_pv[cby][cbx];
					bmap[cby][cbx] = cb_bmap[cby][cbx];
					emap[cby][cbx] = cb_emap[cby][cbx];
				}
		}
#ifdef INTERNAL
		int counterthing;
		if (sdl_key=='v'&&!(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
		{
			if (sdl_mod & (KMOD_SHIFT)) {
				if (vs>=1)
					vs = 0;
				else
					vs = 3;//every other frame
			}
			else
			{
				if (vs>=1)
					vs = 0;
				else
					vs = 1;
			}
			counterthing = 0;
		}
		if (vs)
		{
			if (counterthing+1>=vs)
			{
				dump_frame(vid_buf, XRES, YRES, XRES+BARSIZE);
				counterthing = 0;
			}
			counterthing = (counterthing+1)%3;
		}
#endif

		if (sdl_wheel)
		{
			if (sdl_zoom_trig==1)
			{
				ZSIZE += sdl_wheel;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
				sdl_wheel = 0;
			}
			else
			{
				if (!(sdl_mod & (KMOD_SHIFT|KMOD_CTRL)))
				{
					bsx += sdl_wheel;
					bsy += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_SHIFT) && !(sdl_mod & (KMOD_CTRL)))
				{
					bsx += sdl_wheel;
				}
				else if (sdl_mod & (KMOD_CTRL) && !(sdl_mod & (KMOD_SHIFT)))
				{
					bsy += sdl_wheel;
				}
				if (bsx>1180)
					bsx = 1180;
				if (bsx<0)
					bsx = 0;
				if (bsy>1180)
					bsy = 1180;
				if (bsy<0)
					bsy = 0;
				sdl_wheel = 0;
				/*if(su >= PT_NUM) {
					if(sl < PT_NUM)
						su = sl;
					if(sr < PT_NUM)
						su = sr;
				}*/
			}
		}
		if(console_mode)
		{
			char *console;
			//char error[255] = "error!";
			sys_pause = 1;
			console = console_ui(vid_buf,console_error);
			console = mystrdup(console);
			strcpy(console_error,"");
			if(process_command(vid_buf,console,&console_error)==-1)
			{
				free(console);
				break;
			}
			free(console);
			if(!console_mode)
				hud_enable = 1;
		}

		bq = b;
		b = SDL_GetMouseState(&x, &y);

		for (i=0; i<SC_TOTAL; i++)
		{
			draw_menu(vid_buf, i, active_menu);
		}

		for (i=0; i<SC_TOTAL; i++)
		{
			if (!b&&x>=sdl_scale*(XRES-2) && x<sdl_scale*(XRES+BARSIZE-1) &&y>= sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)) && y<sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)+15))
			{
				active_menu = i;
			}
		}
		menu_ui_v3(vid_buf, active_menu, &sl, &sr, b, bq, x, y);

		if (zoom_en && x>=sdl_scale*zoom_wx && y>=sdl_scale*zoom_wy
		        && x<sdl_scale*(zoom_wx+ZFACTOR*ZSIZE)
		        && y<sdl_scale*(zoom_wy+ZFACTOR*ZSIZE))
		{
			x = (((x/sdl_scale-zoom_wx)/ZFACTOR)+zoom_x)*sdl_scale;
			y = (((y/sdl_scale-zoom_wy)/ZFACTOR)+zoom_y)*sdl_scale;
		}
		if (y>0 && y<sdl_scale*YRES && x>0 && x<sdl_scale*XRES)
		{
			int cr;
			if (photons[y/sdl_scale][x/sdl_scale]) {
				cr = photons[y/sdl_scale][x/sdl_scale];
			} else {
				cr = pmap[y/sdl_scale][x/sdl_scale];
			}
			if (!((cr>>8)>=NPART || !cr))
			{
#ifdef BETA
				if (DEBUG_MODE)
				{
					int tctype = parts[cr>>8].ctype;
					if (tctype>=PT_NUM)
						tctype = 0;
					sprintf(heattext, "%s (%s), Pressure: %3.2f, Temp: %4.2f C, Life: %d", ptypes[cr&0xFF].name, ptypes[tctype].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life);
					//sprintf(heattext, "%s (%s), Pressure: %3.2f, Temp: %4.2f C, Life: %d", ptypes[cr&0xFF].name, ptypes[parts[cr>>8].ctype].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life);
				} else
					sprintf(heattext, "%s, Pressure: %3.2f, Temp: %4.2f C, Life: %d", ptypes[cr&0xFF].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life);
#else
				if (DEBUG_MODE)
				{
					int tctype = parts[cr>>8].ctype;
					if (tctype>=PT_NUM)
						tctype = 0;
					sprintf(heattext, "%s (%s), Pressure: %3.2f, Temp: %4.2f C, Life: %d, #%d", ptypes[cr&0xFF].name, ptypes[tctype].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life ,cr>>8);
					sprintf(coordtext, "X:%d Y:%d", x/sdl_scale, y/sdl_scale);
					//sprintf(heattext, "%s (%s), Pressure: %3.2f, Temp: %4.2f C, Life: %d", ptypes[cr&0xFF].name, ptypes[parts[cr>>8].ctype].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life);
				} else {
					sprintf(heattext, "%s, Pressure: %3.2f, Temp: %4.2f C", ptypes[cr&0xFF].name, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f);
				}
#endif
			}
			else
			{
				if (DEBUG_MODE)
					sprintf(coordtext, "X:%d Y:%d", x/sdl_scale, y/sdl_scale);
				sprintf(heattext, "Empty, Pressure: %3.2f", pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL]);
			}
		}
		mx = x;
		my = y;
		if (update_flag)
		{
			info_box(vid_buf, "Finalizing update...");
			if (last_major>SAVE_VERSION || (last_major==SAVE_VERSION && last_minor>=MINOR_VERSION))
			{
				update_cleanup();
				error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
			}
			else
			{
				if (update_finish())
					error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
				else
					info_ui(vid_buf, "Update success", "You have successfully updated the Powder Toy!");
			}
			update_flag = 0;
		}

		if (b && !bq && x>=(XRES-19-old_ver_len)*sdl_scale &&
		        x<=(XRES-14)*sdl_scale && y>=(YRES-22)*sdl_scale && y<=(YRES-9)*sdl_scale && old_version)
		{
			tmp = malloc(64);
#ifdef BETA
			if (is_beta)
			{
				sprintf(tmp, "Your version: %d (Beta %d), new version: %d (Beta %d).", SAVE_VERSION, MINOR_VERSION, major, minor);
			}
			else
			{
				sprintf(tmp, "Your version: %d (Beta %d), new version: %d.%d.", SAVE_VERSION, MINOR_VERSION, major, minor);
			}
#else
			sprintf(tmp, "Your version: %d.%d, new version: %d.%d.", SAVE_VERSION, MINOR_VERSION, major, minor);
#endif
			if (confirm_ui(vid_buf, "Do you want to update The Powder Toy?", tmp, "Update"))
			{
				free(tmp);
				tmp = download_ui(vid_buf, my_uri, &i);
				if (tmp)
				{
					save_presets(1);
					if (update_start(tmp, i))
					{
						update_cleanup();
						save_presets(0);
						error_ui(vid_buf, 0, "Update failed - try downloading a new version.");
					}
					else
						return 0;
				}
			}
			else
				free(tmp);
		}
		if (y>=sdl_scale*(YRES+(MENUSIZE-20)))
		{
			if (x>=189*sdl_scale && x<=202*sdl_scale && svf_login && svf_open && svf_myvote==0)
			{
				db = svf_own ? 275 : 272;
				if (da < 51)
					da ++;
			}
			else if (x>=204 && x<=217 && svf_login && svf_open && svf_myvote==0)
			{
				db = svf_own ? 275 : 272;
				if (da < 51)
					da ++;
			}
			else if (x>=189 && x<=217 && svf_login && svf_open && svf_myvote!=0)
			{
				db = (svf_myvote==1) ? 273 : 274;
				if (da < 51)
					da ++;
			}
			else if (x>=219*sdl_scale && x<=((XRES+BARSIZE-(510-349))*sdl_scale) && svf_login && svf_open)
			{
				db = svf_own ? 257 : 256;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-351))*sdl_scale) && x<((XRES+BARSIZE-(510-366))*sdl_scale))
			{
				db = 270;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-367))*sdl_scale) && x<((XRES+BARSIZE-(510-383))*sdl_scale))
			{
				db = 266;
				if (da < 51)
					da ++;
			}
			else if (x>=37*sdl_scale && x<=187*sdl_scale && svf_login)
			{
				db = 259;
				if (svf_open && svf_own && x<=55*sdl_scale)
					db = 258;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-385))*sdl_scale) && x<=((XRES+BARSIZE-(510-476))*sdl_scale))
			{
				db = svf_login ? 261 : 260;
				if (svf_admin)
				{
					db = 268;
				}
				else if (svf_mod)
				{
					db = 271;
				}
				if (da < 51)
					da ++;
			}
			else if (x>=sdl_scale && x<=17*sdl_scale)
			{
				db = 262;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-494))*sdl_scale) && x<=((XRES+BARSIZE-(510-509))*sdl_scale))
			{
				db = sys_pause ? 264 : 263;
				if (da < 51)
					da ++;
			}
			else if (x>=((XRES+BARSIZE-(510-476))*sdl_scale) && x<=((XRES+BARSIZE-(510-491))*sdl_scale))
			{
				db = 267;
				if (da < 51)
					da ++;
			}
			else if (x>=19*sdl_scale && x<=35*sdl_scale && svf_open)
			{
				db = 265;
				if (da < 51)
					da ++;
			}
			else if (da > 0)
				da --;
		}
		else if (da > 0)
			da --;

		if (!sdl_zoom_trig && zoom_en==1)
			zoom_en = 0;

		if (sdl_key==Z_keysym && zoom_en==2)
			zoom_en = 1;

		if (load_mode)
		{
			load_x = CELL*((mx/sdl_scale-load_w/2+CELL/2)/CELL);
			load_y = CELL*((my/sdl_scale-load_h/2+CELL/2)/CELL);
			if (load_x+load_w>XRES) load_x=XRES-load_w;
			if (load_y+load_h>YRES) load_y=YRES-load_h;
			if (load_x<0) load_x=0;
			if (load_y<0) load_y=0;
			if (bq==1 && !b)
			{
				parse_save(load_data, load_size, 0, load_x, load_y);
				free(load_data);
				free(load_img);
				load_mode = 0;
			}
			else if (bq==4 && !b)
			{
				free(load_data);
				free(load_img);
				load_mode = 0;
			}
		}
		else if (save_mode==1)
		{
			save_x = (mx/sdl_scale)/CELL;
			save_y = (my/sdl_scale)/CELL;
			if (save_x >= XRES/CELL) save_x = XRES/CELL-1;
			if (save_y >= YRES/CELL) save_y = YRES/CELL-1;
			save_w = 1;
			save_h = 1;
			if (b==1)
			{
				save_mode = 2;
			}
			else if (b==4)
			{
				save_mode = 0;
				copy_mode = 0;
			}
		}
		else if (save_mode==2)
		{
			save_w = (mx/sdl_scale+CELL/2)/CELL - save_x;
			save_h = (my/sdl_scale+CELL/2)/CELL - save_y;
			if (save_w>XRES/CELL) save_w = XRES/CELL;
			if (save_h>YRES/CELL) save_h = YRES/CELL;
			if (save_w<1) save_w = 1;
			if (save_h<1) save_h = 1;
			if (!b)
			{
				if (copy_mode==1)
				{
					clipboard_data=build_save(&clipboard_length, save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
				}
				else if (copy_mode==2)
				{
					clipboard_data=build_save(&clipboard_length, save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
					clear_area(save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
				}
				else if (copy_mode==3)//rotation
				{
					if (save_h>save_w)
						save_w = save_h;
					rotate_area(save_x*CELL, save_y*CELL, save_w*CELL, save_w*CELL,0);//just do squares for now
					save_mode = 0;
					copy_mode = 0;
				}
				else if (copy_mode==4)//invertion
				{
					if (save_h>save_w)
						save_w = save_h;
					rotate_area(save_x*CELL, save_y*CELL, save_w*CELL, save_w*CELL,1);//just do squares for now
					save_mode = 0;
					copy_mode = 0;
				}
				else
				{
					stamp_save(save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
					save_mode = 0;
				}
			}
		}
		else if (sdl_zoom_trig && zoom_en<2)
		{
			x /= sdl_scale;
			y /= sdl_scale;
			x -= ZSIZE/2;
			y -= ZSIZE/2;
			if (x<0) x=0;
			if (y<0) y=0;
			if (x>XRES-ZSIZE) x=XRES-ZSIZE;
			if (y>YRES-ZSIZE) y=YRES-ZSIZE;
			zoom_x = x;
			zoom_y = y;
			zoom_wx = (x<XRES/2) ? XRES-ZSIZE*ZFACTOR : 0;
			zoom_wy = 0;
			zoom_en = 1;
			if (!b && bq)
				zoom_en = 2;
		}
		else if (b)
		{
			if (it > 50)
				it = 50;
			x /= sdl_scale;
			y /= sdl_scale;
			if (y>=YRES+(MENUSIZE-20))
			{
				if (!lb)
				{
					if (x>=189 && x<=202 && svf_login && svf_open && svf_myvote==0 && svf_own==0)
					{
						if (execute_vote(vid_buf, svf_id, "Up"))
						{
							svf_myvote = 1;
						}
					}
					if (x>=204 && x<=217 && svf_login && svf_open && svf_myvote==0 && svf_own==0)
					{
						if (execute_vote(vid_buf, svf_id, "Down"))
						{
							svf_myvote = -1;
						}
					}
					if (x>=219 && x<=(XRES+BARSIZE-(510-349)) && svf_login && svf_open)
						tag_list_ui(vid_buf);
					if (x>=(XRES+BARSIZE-(510-351)) && x<(XRES+BARSIZE-(510-366)) && !bq)
					{
						legacy_enable = !legacy_enable;
					}
					if (x>=(XRES+BARSIZE-(510-367)) && x<=(XRES+BARSIZE-(510-383)) && !bq)
					{
						memset(signs, 0, sizeof(signs));
						memset(pv, 0, sizeof(pv));
						memset(vx, 0, sizeof(vx));
						memset(vy, 0, sizeof(vy));
						memset(fvx, 0, sizeof(fvx));
						memset(fvy, 0, sizeof(fvy));
						memset(bmap, 0, sizeof(bmap));
						memset(emap, 0, sizeof(emap));
						memset(parts, 0, sizeof(particle)*NPART);
						memset(photons, 0, sizeof(photons));
						memset(wireless, 0, sizeof(wireless));
						memset(gol2, 0, sizeof(gol2));
						memset(portal, 0, sizeof(portal));
						for (i=0; i<NPART-1; i++)
							parts[i].life = i+1;
						parts[NPART-1].life = -1;
						pfree = 0;

						legacy_enable = 0;
						svf_myvote = 0;
						svf_open = 0;
						svf_publish = 0;
						svf_own = 0;
						svf_id[0] = 0;
						svf_name[0] = 0;
						svf_tags[0] = 0;
						svf_description[0] = 0;
						gravityMode = 1;
						death = death2 = 0;
						isplayer2 = 0;
						isplayer = 0;
						ISSPAWN1 = 0;
						ISSPAWN2 = 0;

						memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
						memset(fire_r, 0, sizeof(fire_r));
						memset(fire_g, 0, sizeof(fire_g));
						memset(fire_b, 0, sizeof(fire_b));
					}
					if (x>=(XRES+BARSIZE-(510-385)) && x<=(XRES+BARSIZE-(510-476)))
					{
						login_ui(vid_buf);
						if (svf_login)
							save_presets(0);
					}
					if (x>=37 && x<=187 && svf_login)
					{
						if (!svf_open || !svf_own || x>51)
						{
							if (save_name_ui(vid_buf))
								execute_save(vid_buf);
						}
						else
							execute_save(vid_buf);
						while (!sdl_poll())
							if (!SDL_GetMouseState(&x, &y))
								break;
						b = bq = 0;
					}
					if (x>=1 && x<=17)
					{
						search_ui(vid_buf);
						memset(fire_bg, 0, XRES*YRES*PIXELSIZE);
						memset(fire_r, 0, sizeof(fire_r));
						memset(fire_g, 0, sizeof(fire_g));
						memset(fire_b, 0, sizeof(fire_b));
					}
					if (x>=19 && x<=35 && svf_last && svf_open && !bq) {
						//int tpval = sys_pause;
						parse_save(svf_last, svf_lsize, 1, 0, 0);
						//sys_pause = tpval;
					}
					if (x>=(XRES+BARSIZE-(510-476)) && x<=(XRES+BARSIZE-(510-491)) && !bq)
					{
						if (b & SDL_BUTTON_LMASK) {
							set_cmode((cmode+1) % CM_COUNT);
						}
						if (b & SDL_BUTTON_RMASK) {
							if ((cmode+(CM_COUNT-1)) % CM_COUNT == CM_LIFE) {
								set_cmode(CM_GRAD);
							} else {
								set_cmode((cmode+(CM_COUNT-1)) % CM_COUNT);
							}
						}
						save_presets(0);
					}
					if (x>=(XRES+BARSIZE-(510-494)) && x<=(XRES+BARSIZE-(510-509)) && !bq)
						sys_pause = !sys_pause;
					lb = 0;
				}
			}
			else if (y<YRES)
			{
				int signi;
				
				c = (b&1) ? sl : sr;
				su = c;
				
				if(c!=WL_SIGN+100)
				{
					if(!bq)
						for(signi=0; signi<MAXSIGNS; signi++)
							if(sregexp(signs[signi].text, "^{c:[0-9]*|.*}$")==0)
							{
								int signx, signy, signw, signh;
								get_sign_pos(signi, &signx, &signy, &signw, &signh);
								if(x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
								{
									char buff[256];
									int sldr;
									
									memset(buff, 0, sizeof(buff));
									
									for(sldr=3; signs[signi].text[sldr] != '|'; sldr++)
										buff[sldr-3] = signs[signi].text[sldr];
									
									char buff2[sldr-2]; //TODO: Fix this for Visual Studio
									memset(buff2, 0, sizeof(buff2));
									memcpy(&buff2, &buff, sldr-3);
									open_ui(vid_buf, buff2, 0);
								}
							}
				}
				
				if (c==WL_SIGN+100)
				{
					if (!bq)
						add_sign_ui(vid_buf, x, y);
				}
				else if (lb)
				{
					if (lm == 1)
					{
						xor_line(lx, ly, x, y, vid_buf);
						if (c==WL_FAN+100 && lx>=0 && ly>=0 && lx<XRES && ly<YRES && bmap[ly/CELL][lx/CELL]==WL_FAN)
						{
							nfvx = (x-lx)*0.005f;
							nfvy = (y-ly)*0.005f;
							flood_parts(lx, ly, WL_FANHELPER, -1, WL_FAN);
							for (j=0; j<YRES/CELL; j++)
								for (i=0; i<XRES/CELL; i++)
									if (bmap[j][i] == WL_FANHELPER)
									{
										fvx[j][i] = nfvx;
										fvy[j][i] = nfvy;
										bmap[j][i] = WL_FAN;
									}
						}
					}
					else if (lm == 2)
					{
						xor_line(lx, ly, lx, y, vid_buf);
						xor_line(lx, y, x, y, vid_buf);
						xor_line(x, y, x, ly, vid_buf);
						xor_line(x, ly, lx, ly, vid_buf);
					}
					else
					{
						create_line(lx, ly, x, y, bsx, bsy, c);
						lx = x;
						ly = y;
					}
				}
				else
				{
					if ((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT)) && !(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL|KMOD_LALT)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 1;
					}
					else if ((sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)) && !(sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 2;
					}
					else if ((sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)) && (sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT)) && !(sdl_mod & (KMOD_LALT)))
					{
						if (sdl_mod & (KMOD_CAPS))
							c = 0;
						if (c!=WL_STREAM+100&&c!=SPC_AIR&&c!=SPC_HEAT&&c!=SPC_COOL&&c!=SPC_VACUUM&&!REPLACE_MODE)
							flood_parts(x, y, c, -1, -1);
						if (c==SPC_HEAT || c==SPC_COOL)
							create_parts(x, y, bsx, bsy, c);
						lx = x;
						ly = y;
						lb = 0;
						lm = 0;
					}
					else if (((sdl_mod & (KMOD_LALT|KMOD_RALT)) && !(sdl_mod & (KMOD_SHIFT))) || b==SDL_BUTTON_MIDDLE)
					{
						if (y>0 && y<sdl_scale*YRES && x>0 && x<sdl_scale*XRES)
						{
							int cr;
							cr = pmap[y][x];
							if (!((cr>>8)>=NPART || !cr))
							{
								c = sl = cr&0xFF;
							}
							else
							{
								//Something
							}
						}
						lx = x;
						ly = y;
						lb = 0;
						lm = 0;
					}
					else
					{
						//Copy state before drawing any particles (for undo)7
						int cbx, cby, cbi;

						for (cbi=0; cbi<NPART; cbi++)
							cb_parts[cbi] = parts[cbi];

						for (cby = 0; cby<YRES; cby++)
							for (cbx = 0; cbx<XRES; cbx++)
								cb_pmap[cby][cbx] = pmap[cby][cbx];

						for (cby = 0; cby<(YRES/CELL); cby++)
							for (cbx = 0; cbx<(XRES/CELL); cbx++)
							{
								cb_vx[cby][cbx] = vx[cby][cbx];
								cb_vy[cby][cbx] = vy[cby][cbx];
								cb_pv[cby][cbx] = pv[cby][cbx];
								cb_bmap[cby][cbx] = bmap[cby][cbx];
								cb_emap[cby][cbx] = emap[cby][cbx];
							}
						create_parts(x, y, bsx, bsy, c);
						lx = x;
						ly = y;
						lb = b;
						lm = 0;
					}
				}
			}
		}
		else
		{
			if (lb && lm)
			{
				x /= sdl_scale;
				y /= sdl_scale;
				c = (lb&1) ? sl : sr;
				su = c;
				if (lm == 1)
				{
					if (c!=WL_FAN+100 || lx<0 || ly<0 || lx>=XRES || ly>=YRES || bmap[ly/CELL][lx/CELL]!=WL_FAN)
						create_line(lx, ly, x, y, bsx, bsy, c);
				}
				else
					create_box(lx, ly, x, y, c);
				lm = 0;
			}
			lb = 0;
		}

		if (load_mode)
		{
			draw_image(vid_buf, load_img, load_x, load_y, load_w, load_h, 128);
			xor_rect(vid_buf, load_x, load_y, load_w, load_h);
		}

		if (save_mode)
		{
			if (copy_mode==3||copy_mode==4)//special drawing for rotate, can remove once it can do rectangles
			{
				if (save_h>save_w)
					save_w = save_h;
				xor_rect(vid_buf, save_x*CELL, save_y*CELL, save_w*CELL, save_w*CELL);
			}
			else
				xor_rect(vid_buf, save_x*CELL, save_y*CELL, save_w*CELL, save_h*CELL);
			da = 51;
			db = 269;
		}

		if (zoom_en!=1 && !load_mode && !save_mode)
		{
			render_cursor(vid_buf, mx/sdl_scale, my/sdl_scale, su, bsx, bsy);
			mousex = mx/sdl_scale;
			mousey = my/sdl_scale;
		}

		if (zoom_en)
			render_zoom(vid_buf);

		if (da)
			switch (db)
			{
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
			case 270:
				drawtext(vid_buf, 16, YRES-24, "Enable or disable compatibility mode (disables heat simulation).", 255, 255, 255, da*5);
				break;
			case 271:
				drawtext(vid_buf, 16, YRES-24, "You're a moderator", 255, 255, 255, da*5);
				break;
			case 272:
				drawtext(vid_buf, 16, YRES-24, "Like/Dislike this save.", 255, 255, 255, da*5);
				break;
			case 273:
				drawtext(vid_buf, 16, YRES-24, "You like this.", 255, 255, 255, da*5);
				break;
			case 274:
				drawtext(vid_buf, 16, YRES-24, "You dislike this.", 255, 255, 255, da*5);
				break;
			case 275:
				drawtext(vid_buf, 16, YRES-24, "You cannot vote on your own save.", 255, 255, 255, da*5);
				break;
			default:
				drawtext(vid_buf, 16, YRES-24, (char *)ptypes[db].descs, 255, 255, 255, da*5);
			}
		if (itc)
		{
			itc--;
			drawtext(vid_buf, (XRES-textwidth(itc_msg))/2, ((YRES/2)-10), itc_msg, 255, 255, 255, itc>51?255:itc*5);
		}
		if (it)
		{
			it--;
			drawtext(vid_buf, 16, 20, it_msg, 255, 255, 255, it>51?255:it*5);
		}

		if (old_version)
		{
			clearrect(vid_buf, XRES-21-old_ver_len, YRES-24, old_ver_len+9, 17);
#ifdef BETA
			if (is_beta)
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg_beta, 255, 216, 32, 255);
			}
			else
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
			}
#else
			drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
#endif
			drawrect(vid_buf, XRES-19-old_ver_len, YRES-22, old_ver_len+5, 13, 255, 216, 32, 255);
		}

		if (hud_enable)
		{
			currentTime = SDL_GetTicks();
			if (currentTime-past>=16)
			{
				past = SDL_GetTicks();
				FPS++;
			}
			if (currentTime-pastFPS>=1000)
			{
				FPSB = FPS;
				FPS = 0;
				pastFPS = currentTime;
			}

#ifdef BETA
			sprintf(uitext, "Version %d Beta %d FPS:%d Parts:%d Generation:%d", SAVE_VERSION, MINOR_VERSION, FPSB, NUM_PARTS,GENERATION);
#else
			if (DEBUG_MODE)
				sprintf(uitext, "Version %d.%d FPS:%d Parts:%d Generation:%d", SAVE_VERSION, MINOR_VERSION, FPSB, NUM_PARTS,GENERATION);
			else
				sprintf(uitext, "Version %d.%d FPS:%d", SAVE_VERSION, MINOR_VERSION, FPSB);
#endif
			if (REPLACE_MODE)
				strappend(uitext, " [REPLACE MODE]");
			if (sdl_mod&(KMOD_CAPS))
				strappend(uitext, " [CAP LOCKS]");
			if (GRID_MODE)
				sprintf(uitext, "%s [GRID: %d]", uitext, GRID_MODE);
#ifdef INTERNAL
			if (vs)
				strappend(uitext, " [FRAME CAPTURE]");
#endif

			if (sdl_zoom_trig||zoom_en)
			{
				if (zoom_x<XRES/2)
				{
					fillrect(vid_buf, XRES-20-textwidth(heattext), 266, textwidth(heattext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, XRES-16-textwidth(heattext), 270, heattext, 255, 255, 255, 200);
					if(DEBUG_MODE)
					{
						fillrect(vid_buf, XRES-20-textwidth(coordtext), 280, textwidth(coordtext)+8, 13, 0, 0, 0, 140);
						drawtext(vid_buf, XRES-16-textwidth(coordtext), 282, coordtext, 255, 255, 255, 200);
					}
				}
				else
				{
					fillrect(vid_buf, 12, 266, textwidth(heattext)+8, 15, 0, 0, 0, 140);
					drawtext(vid_buf, 16, 270, heattext, 255, 255, 255, 200);
					if(DEBUG_MODE)
					{
						fillrect(vid_buf, 12, 280, textwidth(coordtext)+8, 13, 0, 0, 0, 140);
						drawtext(vid_buf, 16, 282, coordtext, 255, 255, 255, 200);
					}
				}
			}
			else
			{
				fillrect(vid_buf, XRES-20-textwidth(heattext), 12, textwidth(heattext)+8, 15, 0, 0, 0, 140);
				drawtext(vid_buf, XRES-16-textwidth(heattext), 16, heattext, 255, 255, 255, 200);
				if(DEBUG_MODE)
				{
					fillrect(vid_buf, XRES-20-textwidth(coordtext), 26, textwidth(coordtext)+8, 11, 0, 0, 0, 140);
					drawtext(vid_buf, XRES-16-textwidth(coordtext), 27, coordtext, 255, 255, 255, 200);	
				}
			}
			fillrect(vid_buf, 12, 12, textwidth(uitext)+8, 15, 0, 0, 0, 140);
			drawtext(vid_buf, 16, 16, uitext, 32, 216, 255, 200);
		}
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf, XRES+BARSIZE);

		//Setting an element for the stick man
		if (isplayer==0)
		{
			if (ptypes[sr].falldown>0 || sr == PT_NEUT || sr == PT_PHOT)
				player[2] = sr;
			else
				player[2] = PT_DUST;
		}
		if (isplayer2==0)
		{
			if (ptypes[sr].falldown>0 || sr == PT_NEUT || sr == PT_PHOT)
				player2[2] = sr;
			else
				player2[2] = PT_DUST;
		}

	}
	SDL_CloseAudio();
	http_done();
	return 0;
}
int process_command(pixel *vid_buf,char *console,char *console_error) {
	int y,x,nx,ny,i,j,k,m;
	int do_next = 1;
	char xcoord[10];
	char ycoord[10];
	char console2[15];
	char console3[15];
	char console4[15];
	char console5[15];
	//sprintf(console_error, "%s", console);
	if(console && strcmp(console, "")!=0 && strncmp(console, " ", 1)!=0)
	{
		sscanf(console,"%14s %14s %14s %14s", console2, console3, console4, console5);//why didn't i know about this function?!
		if(strcmp(console2, "quit")==0)
		{
			return -1;
		}
		else if(strcmp(console2, "file")==0 && console3)
		{
			FILE *f=fopen(console3, "r");
			if(f)
			{
				nx = 0;
				ny = 0;
				j = 0;
				m = 0;
				if(console4)
					console_parse_coords(console4, &nx , &ny, console_error);
				char fileread[5000];//TODO: make this change with file size
				char pch[5000];
				memset(pch,0,sizeof(pch));
				memset(fileread,0,sizeof(fileread));
				char tokens[10];
				int tokensize;
				fread(fileread,1,5000,f);
				for(i=0; i<strlen(fileread); i++)
				{
					if(fileread[i] != '\n')
					{
						pch[i-j] = fileread[i];
						if(fileread[i] != ' ')
							tokens[i-m] = fileread[i];
					}
					if(fileread[i] == ' ' || fileread[i] == '\n')
					{
						if(sregexp(tokens,"^x.[0-9],y.[0-9]")==0)//TODO: fix regex matching to work with x,y ect, right now it has to have a +0 or -0
						{
							char temp[5];
							int starty = 0;
							tokensize = strlen(tokens);
							x = 0;
							y = 0;
							sscanf(tokens,"x%d,y%d",&x,&y);
							sscanf(tokens,"%9s,%9s",xcoord,ycoord);
							x += nx;
							y += ny;
							sprintf(xcoord,"%d",x);
							sprintf(ycoord,"%d",y);
							for(k = 0; k<strlen(xcoord);k++)//rewrite pch with numbers
							{
								pch[i-j-tokensize+k] = xcoord[k];
								starty = k+1;
							}
							pch[i-j-tokensize+starty] = ',';
							starty++;
							for(k=0;k<strlen(ycoord);k++)
							{
								pch[i-j-tokensize+starty+k] = ycoord[k];
								
							}
							pch[i-j-tokensize +strlen(xcoord) +1 +strlen(ycoord)] = ' ';
							j = j -tokensize +strlen(xcoord) +1 +strlen(ycoord);
						}
						memset(tokens,0,sizeof(tokens));
						m = i+1;
					}
					if(fileread[i] == '\n')
					{
						
						if(do_next)
						{
							if(strcmp(pch,"else")==0)
								do_next = 0;
							else
								do_next = process_command(vid_buf, pch, console_error);
						}
						else if(strcmp(pch,"endif")==0 || strcmp(pch,"else")==0)
							do_next = 1;
						memset(pch,0,sizeof(pch));
						j = i+1;
					}
				}
				//sprintf(console_error, "%s exists", console3);
				fclose(f);
			}
			else
				sprintf(console_error, "%s does not exist", console3);
		}
		else if(strcmp(console2, "sound")==0 && console3)
		{
			play_sound(console3);
		}
		else if(strcmp(console2, "load")==0 && console3)
		{
			j = atoi(console3);
			if(j)
			{
				open_ui(vid_buf, console3, NULL);
				console_mode = 0;
			}
		}
		else if(strcmp(console2, "if")==0 && console3)
		{
			if(strcmp(console3, "type")==0)//TODO: add more than just type, and be able to check greater/less than
			{
				if (console_parse_partref(console4, &i, console_error)
					&& console_parse_type(console5, &j, console_error))
				{
					if(parts[i].type==j)
						return 1;
					else
						return 0;
				}
				else
					return 0;
			}
		}
		else if (strcmp(console2, "create")==0 && console3 && console4)
		{
			if (console_parse_type(console3, &j, console_error)
				&& console_parse_coords(console4, &nx, &ny, console_error))
			{
				if (!j)
					strcpy(console_error, "Cannot create particle with type NONE");
				else if (create_part(-1,nx,ny,j)<0)
					strcpy(console_error, "Could not create particle");
			}
		}
		else if ((strcmp(console2, "delete")==0 || strcmp(console2, "kill")==0) && console3)
		{
			if (console_parse_partref(console3, &i, console_error))
				kill_part(i);
		}
		else if(strcmp(console2, "reset")==0 && console3)
		{
			if(strcmp(console3, "pressure")==0)
			{
				for (nx = 0; nx<XRES/CELL; nx++)
					for (ny = 0; ny<YRES/CELL; ny++)
					{
						pv[ny][nx] = 0;
					}
			}
			else if(strcmp(console3, "velocity")==0)
			{
				for (nx = 0; nx<XRES/CELL; nx++)
					for (ny = 0; ny<YRES/CELL; ny++)
					{
						vx[ny][nx] = 0;
						vy[ny][nx] = 0;
					}
			}
			else if(strcmp(console3, "sparks")==0)
			{
				for(i=0; i<NPART; i++)
				{
					if(parts[i].type==PT_SPRK)
					{
						parts[i].type = parts[i].ctype;
						parts[i].life = 4;
					}
				}
			}
			else if(strcmp(console3, "temp")==0)
			{
				for(i=0; i<NPART; i++)
				{
					if(parts[i].type)
					{
						parts[i].temp = ptypes[parts[i].type].heat;
					}
				}
			}
		}
		else if(strcmp(console2, "set")==0 && console3 && console4 && console5)
		{
			if(strcmp(console3, "life")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].life = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].life = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].life = j;
					}
				}
			}
			if(strcmp(console3, "type")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					if (console_parse_type(console5, &j, console_error))
						for(i=0; i<NPART; i++)
						{
							if(parts[i].type)
								parts[i].type = j;
						}
				}
				else if (console_parse_type(console4, &j, console_error)
					     && console_parse_type(console5, &k, console_error))
				{
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].type = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error)
						&& console_parse_type(console5, &j, console_error))
					{
						parts[i].type = j;
					}
				}
			}
			if(strcmp(console3, "temp")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].temp = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].temp= k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].temp = j;
					}
				}
			}
			if(strcmp(console3, "tmp")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].tmp = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].tmp = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].tmp = j;
					}
				}
			}
			if(strcmp(console3, "x")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].x = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].x = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].x = j;
					}
				}
			}
			if(strcmp(console3, "y")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].y = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].y = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].y = j;
					}
				}
			}
			if(strcmp(console3, "ctype")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					if (console_parse_type(console5, &j, console_error))
						for(i=0; i<NPART; i++)
						{
							if(parts[i].type)
								parts[i].ctype = j;
						}
				}
				else if (console_parse_type(console4, &j, console_error)
					     && console_parse_type(console5, &k, console_error))
				{
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].ctype = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error)
						&& console_parse_type(console5, &j, console_error))
					{
						parts[i].ctype = j;
					}
				}
			}
			if(strcmp(console3, "vx")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].vx = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].vx = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].vx = j;
					}
				}
			}
			if(strcmp(console3, "vy")==0)
			{
				if(strcmp(console4, "all")==0)
				{
					j = atoi(console5);
					for(i=0; i<NPART; i++)
					{
						if(parts[i].type)
							parts[i].vy = j;
					}
				}
				else if (console_parse_type(console4, &j, console_error))
				{
					k = atoi(console5);
					for(i=0; i<NPART; i++)
						{
							if(parts[i].type == j)
								parts[i].vy = k;
						}
				}
				else
				{
					if (console_parse_partref(console4, &i, console_error))
					{
						j = atoi(console5);
						parts[i].vy = j;
					}
				}
			}
		}
		else
			sprintf(console_error, "Invalid Command", console2);
	}
	return 1;
}

