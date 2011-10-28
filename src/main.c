/**
 * Powder Toy - Main source
 *
 * Copyright (c) 2008 - 2011 Stanislaw Skowronek.
 * Copyright (c) 2010 - 2011 Simon Robertshaw
 * Copyright (c) 2010 - 2011 Skresanov Savely
 * Copyright (c) 2010 - 2011 Bryan Hoyle
 * Copyright (c) 2010 - 2011 Nathan Cousins
 * Copyright (c) 2010 - 2011 cracker64
 * Copyright (c) 2011 jacksonmj
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

#include <defines.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include <bzlib.h>
#include <time.h>
#include <pthread.h>

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <misc.h>
#include <font.h>
#include <powder.h>
#include <graphics.h>
#include <version.h>
#include <http.h>
#include <md5.h>
#include <update.h>
#include <hmap.h>
#include <air.h>
#include <icon.h>
#include <console.h>
#ifdef LUACONSOLE
#include "luaconsole.h"
#endif

pixel *vid_buf;

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

//plays a .wav file (sounds must be enabled)
void play_sound(char *file)
{
	int index;
	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;

	if (!sound_enable) return;

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
    "\blThe Powder Toy - Version " MTOS(SAVE_VERSION) "." MTOS(MINOR_VERSION) " - http://powdertoy.co.uk, irc.freenode.net #powder\n"
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
    "Contributors: \bgStanislaw K Skowronek (\brhttp://powder.unaligned.org\bg, \bbirc.unaligned.org #wtf\bg),\n"
    "\bgSimon Robertshaw, Skresanov Savely, cracker64, Catelite, Bryan Hoyle, Nathan Cousins, jacksonmj,\n"
	"\bgLieuwe Mosch, Anthony Boot, Matthew Miller, MaksProg\n"
    "\n"
    "\bgTo use online features such as saving, you need to register at: \brhttp://powdertoy.co.uk/Register.html"
    ;

typedef struct
{
	int start, inc;
	pixel *vid;
} upstruc;

static const char *old_ver_msg_beta = "A new beta is available - click here!";
static const char *old_ver_msg = "A new version is available - click here!";
char new_message_msg[255];
float mheat = 0.0f;

int do_open = 0;
int sys_pause = 0;
int sys_shortcuts = 1;
int legacy_enable = 0; //Used to disable new features such as heat, will be set by save.
int ngrav_enable = 0; //Newtonian gravity, will be set by save
int aheat_enable; //Ambient heat
int decorations_enable = 1;
int hud_enable = 1;
int active_menu = 0;
int framerender = 0;
int pretty_powder = 0;
int amd = 1;
int FPSB = 0;
int MSIGN =-1;
int frameidx = 0;
//int CGOL = 0;
//int GSPEED = 1;//causes my .exe to crash..
int sound_enable = 0;

int debug_flags = 0;
int debug_perf_istart = 1;
int debug_perf_iend = 0;
long debug_perf_frametime[DEBUG_PERF_FRAMECOUNT];
long debug_perf_partitime[DEBUG_PERF_FRAMECOUNT];
long debug_perf_time = 0;

sign signs[MAXSIGNS];

int numCores = 4;

pthread_t gravthread;
pthread_mutex_t gravmutex;
pthread_cond_t gravcv;
int grav_ready = 0;
int gravthread_done = 0;

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
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(app_icon, 16, 16, 32, 64, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
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

//the saving function
void *build_save(int *size, int orig_x0, int orig_y0, int orig_w, int orig_h, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr)
{
	unsigned char *d=calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*15+MAXSIGNS*262), *c;
	int i,j,x,y,p=0,*m=calloc(XRES*YRES, sizeof(int));
	int x0, y0, w, h, bx0=orig_x0/CELL, by0=orig_y0/CELL, bw, bh;
	particle *parts = partsptr;
	bw=(orig_w+orig_x0-bx0*CELL+CELL-1)/CELL;
	bh=(orig_h+orig_y0-by0*CELL+CELL-1)/CELL;

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
			if (x>=orig_x0 && x<orig_x0+orig_w && y>=orig_y0 && y<orig_y0+orig_h) {
				if (!m[(x-x0)+(y-y0)*w] ||
				        parts[m[(x-x0)+(y-y0)*w]-1].type == PT_PHOT ||
				        parts[m[(x-x0)+(y-y0)*w]-1].type == PT_NEUT)
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
		if (i && (parts[i-1].type==PT_PBCN)) {
			//Save tmp2
			d[p++] = parts[i-1].tmp2;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (ALPHA)
			d[p++] = (parts[i-1].dcolour&0xFF000000)>>24;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (RED)
			d[p++] = (parts[i-1].dcolour&0x00FF0000)>>16;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (GREEN)
			d[p++] = (parts[i-1].dcolour&0x0000FF00)>>8;
		}
	}
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i) {
			//Save colour (BLUE)
			d[p++] = (parts[i-1].dcolour&0x000000FF);
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
		if (i && (parts[i-1].type==PT_CLNE || parts[i-1].type==PT_PCLN || parts[i-1].type==PT_BCLN || parts[i-1].type==PT_SPRK || parts[i-1].type==PT_LAVA || parts[i-1].type==PT_PIPE || parts[i-1].type==PT_LIFE || parts[i-1].type==PT_PBCN || parts[i-1].type==PT_WIRE || parts[i-1].type==PT_STOR || parts[i-1].type==PT_CONV))
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
	c[3] = legacy_enable|((sys_pause<<1)&0x02)|((gravityMode<<2)&0x0C)|((airMode<<4)&0x70)|((ngrav_enable<<7)&0x80);
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
		free(m);
		return NULL;
	}
	free(d);
	free(m);

	*size = i+12;
	return c;
}

int parse_save(void *save, int size, int replace, int x0, int y0, unsigned char bmap[YRES/CELL][XRES/CELL], float fvx[YRES/CELL][XRES/CELL], float fvy[YRES/CELL][XRES/CELL], sign signs[MAXSIGNS], void* partsptr, unsigned pmap[YRES][XRES])
{
	unsigned char *d=NULL,*c=save;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0, tempGrav = 0;
	int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
	int nf=0, new_format = 0, ttv = 0;
	particle *parts = partsptr;
	int *fp = malloc(NPART*sizeof(int));
	parts_lastActiveIndex = NPART-1;

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
			if (ver>=46 && replace) {
				gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49 && replace) {
				tempGrav = ((c[3]>>7)&0x01);		
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
		if (ver<46) {
			gravityMode = 0;
			airMode = 0;
		}
		clear_sim();
	}
	m = calloc(XRES*YRES, sizeof(int));

	// make a catalog of free parts
	//memset(pmap, 0, sizeof(pmap)); "Using sizeof for array given as function argument returns the size of pointer."
	memset(pmap, 0, sizeof(unsigned)*(XRES*YRES));
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
			if (j)
			{
				if (pmap[y][x])
				{
					k = pmap[y][x]>>8;
					memset(parts+k, 0, sizeof(particle));
					parts[k].type = j;
					if (j == PT_PHOT)
						parts[k].ctype = 0x3fffffff;
					if (j == PT_SOAP)
						parts[k].ctype = 0;
					parts[k].x = (float)x;
					parts[k].y = (float)y;
					m[(x-x0)+(y-y0)*w] = k+1;
				}
				else if (i < nf)
				{
					memset(parts+fp[i], 0, sizeof(particle));
					parts[fp[i]].type = j;
					if (j == PT_COAL)
						parts[fp[i]].tmp = 50;
					if (j == PT_FUSE)
						parts[fp[i]].tmp = 50;
					if (j == PT_PHOT)
						parts[fp[i]].ctype = 0x3fffffff;
					if (j == PT_SOAP)
						parts[fp[i]].ctype = 0;
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
					if (ver<53 && !parts[i-1].tmp)
						for (q = 1; q<=NGOLALT; q++) {
							if (parts[i-1].type==goltype[q-1] && grule[q][9]==2)
								parts[i-1].tmp = grule[q][9]-1;
						}
					if (ver>=51 && ver<53 && parts[i-1].type==PT_PBCN)
					{
						parts[i-1].tmp2 = parts[i-1].tmp;
						parts[i-1].tmp = 0;
					}
				} else {
					p+=2;
				}
			}
		}
	}
	if (ver>=53) {
		for (j=0; j<w*h; j++)
		{
			i = m[j];
			ty = d[pty+j];
			if (i && ty==PT_PBCN)
			{
				if (p >= size)
					goto corrupt;
				if (i <= NPART)
					parts[i-1].tmp2 = d[p++];
				else
					p++;
			}
		}
	}
	//Read ALPHA component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour = d[p++]<<24;
				} else {
					p++;
				}
			}
		}
	}
	//Read RED component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<16;
				} else {
					p++;
				}
			}
		}
	}
	//Read GREEN component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++]<<8;
				} else {
					p++;
				}
			}
		}
	}
	//Read BLUE component
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			if (ver>=49) {
				if (p >= size) {
					goto corrupt;
				}
				if (i <= NPART) {
					parts[i-1].dcolour |= d[p++];
				} else {
					p++;
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
		int gnum = 0;
		i = m[j];
		ty = d[pty+j];
		if (i && (ty==PT_CLNE || (ty==PT_PCLN && ver>=43) || (ty==PT_BCLN && ver>=44) || (ty==PT_SPRK && ver>=21) || (ty==PT_LAVA && ver>=34) || (ty==PT_PIPE && ver>=43) || (ty==PT_LIFE && ver>=51) || (ty==PT_PBCN && ver>=52) || (ty==PT_WIRE && ver>=55) || (ty==PT_STOR && ver>=59) || (ty==PT_CONV && ver>=60)))
		{
			if (p >= size)
				goto corrupt;
			if (i <= NPART)
				parts[i-1].ctype = d[p++];
			else
				p++;
		}
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if ((player.spwn == 1 && ty==PT_STKM) || (player2.spwn == 1 && ty==PT_STKM2))
			{
				parts[i-1].type = PT_NONE;
			}
			else if (parts[i-1].type == PT_STKM)
			{
				STKM_init_legs(&player, i-1);
				player.spwn = 1;
				player.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_STKM2)
			{
				STKM_init_legs(&player2, i-1);
				player2.spwn = 1;
				player2.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_FIGH)
			{
				unsigned char fcount = 0;
				while (fcount < 100 && fcount < (fighcount+1) && fighters[fcount].spwn==1) fcount++;
				if (fcount < 100 && fighters[fcount].spwn==0)
				{
					parts[i-1].tmp = fcount;
					fighters[fcount].spwn = 1;
					fighters[fcount].elem = PT_DUST;
					fighcount++;
					STKM_init_legs(&(fighters[fcount]), i-1);
				}
			}

			if (ver<48 && (ty==OLD_PT_WIND || (ty==PT_BRAY&&parts[i-1].life==0)))
			{
				// Replace invisible particles with something sensible and add decoration to hide it
				x = (int)(parts[i-1].x+0.5f);
				y = (int)(parts[i-1].y+0.5f);
				parts[i-1].dcolour = 0xFF000000;
				parts[i-1].type = PT_DMND;
			}
			if(ver<51 && ((ty>=78 && ty<=89) || (ty>=134 && ty<=146 && ty!=141))){
				//Replace old GOL
				parts[i-1].type = PT_LIFE;
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (ty==goltype[gnum])
						parts[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (parts[i-1].ctype==goltype[gnum])
					{
						parts[i-1].ctype = PT_LIFE;
						parts[i-1].tmp = gnum;
					}
				}
			}
			if (!ptypes[parts[i-1].type].enabled)
				parts[i-1].type = PT_NONE;
		}
	}

	#ifndef RENDERER
	//Change the gravity state
	if(ngrav_enable != tempGrav && replace)
	{
		if(tempGrav)
			start_grav_async();
		else
			stop_grav_async();
	}
	#endif
	
	gravity_mask();

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
			clean_text(signs[k].text, 158-14 /* Current max sign length */);
		}
		p += x;
	}

version1:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);

	return 0;

corrupt:
	if (m) free(m);
	if (d) free(d);
	if (fp) free(fp);
	if (replace)
	{
		legacy_enable = 0;
		clear_sim();
	}
	return 1;
}

void clear_sim(void)
{
	int x, y;
	memset(bmap, 0, sizeof(bmap));
	memset(emap, 0, sizeof(emap));
	memset(signs, 0, sizeof(signs));
	memset(parts, 0, sizeof(particle)*NPART);
	pfree = -1;
	parts_lastActiveIndex = NPART-1;
	memset(pmap, 0, sizeof(pmap));
	memset(pv, 0, sizeof(pv));
	memset(vx, 0, sizeof(vx));
	memset(vy, 0, sizeof(vy));
	memset(fvx, 0, sizeof(fvx));
	memset(fvy, 0, sizeof(fvy));
	memset(photons, 0, sizeof(photons));
	memset(wireless, 0, sizeof(wireless));
	memset(gol2, 0, sizeof(gol2));
	memset(portalp, 0, sizeof(portalp));
	memset(fighters, 0, sizeof(fighters));
	fighcount = 0;
	ISSPAWN1 = ISSPAWN2 = 0;
	player.spwn = 0;
	player2.spwn = 0;
	memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
	memset(fire_r, 0, sizeof(fire_r));
	memset(fire_g, 0, sizeof(fire_g));
	memset(fire_b, 0, sizeof(fire_b));
	memset(gravmask, 0xFF, sizeof(gravmask));
	memset(gravy, 0, sizeof(gravy));
	memset(gravx, 0, sizeof(gravx));
	for(x = 0; x < XRES/CELL; x++){
		for(y = 0; y < YRES/CELL; y++){
			hv[y][x] = 273.15f+22.0f; //Set to room temperature
		}
	}
	gravity_mask();
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
	void *s=build_save(&n, x, y, w, h, bmap, fvx, fvy, signs, parts);

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

unsigned char last_major=0, last_minor=0, last_build=0, update_flag=0;

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

int set_scale(int scale, int kiosk){
	int old_scale = sdl_scale, old_kiosk = kiosk_enable;
	sdl_scale = scale;
	kiosk_enable = kiosk;
	if (!sdl_open())
	{
		sdl_scale = old_scale;
		kiosk_enable = old_kiosk;
		sdl_open();
		return 0;
	}
	return 1;
}
				
void* update_grav_async(void* unused)
{
	int done = 0;
	int thread_done = 0;
	memset(th_ogravmap, 0, sizeof(th_ogravmap));
	memset(th_gravmap, 0, sizeof(th_gravmap));
	memset(th_gravy, 0, sizeof(th_gravy));
	memset(th_gravx, 0, sizeof(th_gravx));
#ifdef GRAVFFT
	grav_fft_init();
#endif
	while(!thread_done){
		if(!done){
			update_grav();
			done = 1;
			pthread_mutex_lock(&gravmutex);
			
			grav_ready = done;
			thread_done = gravthread_done;
			
			pthread_mutex_unlock(&gravmutex);
		} else {
			pthread_mutex_lock(&gravmutex);
			pthread_cond_wait(&gravcv, &gravmutex);
		    
			done = grav_ready;
			thread_done = gravthread_done;
			
			pthread_mutex_unlock(&gravmutex);
		}
	}
	pthread_exit(NULL);
}

void start_grav_async()
{
	if(!ngrav_enable){
		gravthread_done = 0;
		grav_ready = 0;
		pthread_mutex_init (&gravmutex, NULL);
		pthread_cond_init(&gravcv, NULL);
		pthread_create(&gravthread, NULL, update_grav_async, NULL); //Start asynchronous gravity simulation
		ngrav_enable = 1;
	}
	memset(gravyf, 0, sizeof(gravyf));
	memset(gravxf, 0, sizeof(gravxf));
	memset(gravpf, 0, sizeof(gravpf));
}

void stop_grav_async()
{
	if(ngrav_enable){
		pthread_mutex_lock(&gravmutex);
		gravthread_done = 1;
		pthread_cond_signal(&gravcv);
		pthread_mutex_unlock(&gravmutex);
		pthread_join(gravthread, NULL);
		pthread_mutex_destroy(&gravmutex); //Destroy the mutex
		memset(gravy, 0, sizeof(gravy)); //Clear the grav velocities
		memset(gravx, 0, sizeof(gravx)); //Clear the grav velocities
		ngrav_enable = 0;
	}
	memset(gravyf, 0, sizeof(gravyf));
	memset(gravxf, 0, sizeof(gravxf));
	memset(gravpf, 0, sizeof(gravpf));
}

#ifdef RENDERER
int main(int argc, char *argv[])
{
	pixel *vid_buf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	int load_size, i=0, j=0;
	void *load_data = file_load(argv[1], &load_size);
	unsigned char c[3];
	char ppmfilename[256], ptifilename[256], ptismallfilename[256];
	FILE *f;
	
	cmode = CM_FIRE;
	sys_pause = 1;
	parts = calloc(sizeof(particle), NPART);
	for (i=0; i<NPART-1; i++)
		parts[i].life = i+1;
	parts[NPART-1].life = -1;
	pfree = 0;

	pers_bg = calloc((XRES+BARSIZE)*YRES, PIXELSIZE);
	
	prepare_alpha(4, 1.0f);
	player.elem = player2.elem = PT_DUST;
	player.frames = player2.frames = 0;

	sprintf(ppmfilename, "%s.ppm", argv[2]);
	sprintf(ptifilename, "%s.pti", argv[2]);
	sprintf(ptismallfilename, "%s-small.pti", argv[2]);
	
	if(load_data && load_size){
		int parsestate = 0;
		//parsestate = parse_save(load_data, load_size, 1, 0, 0);
		parsestate = parse_save(load_data, load_size, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap);
		
		for(i=0; i<30; i++){
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
			draw_walls(vid_buf);
			update_particles(vid_buf);
			render_parts(vid_buf);
			render_fire(vid_buf);
		}
		
		render_signs(vid_buf);
		
		if(parsestate>0){
			//return 0;
			info_box(vid_buf, "Save file invalid or from newer version");
		}

		//Save PTi images
		char * datares = NULL, *scaled_buf;
		int res = 0, sw, sh;
		datares = ptif_pack(vid_buf, XRES, YRES, &res);
		if(datares!=NULL){
			f=fopen(ptifilename, "wb");
			fwrite(datares, res, 1, f);
			fclose(f);
			free(datares);
			datares = NULL;
		}
		scaled_buf = resample_img(vid_buf, XRES, YRES, XRES/GRID_Z, YRES/GRID_Z);
		datares = ptif_pack(scaled_buf, XRES/GRID_Z, YRES/GRID_Z, &res);
		if(datares!=NULL){
			f=fopen(ptismallfilename, "wb");
			fwrite(datares, res, 1, f);
			fclose(f);
			free(datares);
			datares = NULL;
		}
		free(scaled_buf);
		//Save PPM image
		f=fopen(ppmfilename, "wb");
		fprintf(f,"P6\n%d %d\n255\n",XRES,YRES);
		for (j=0; j<YRES; j++)
		{
			for (i=0; i<XRES; i++)
			{
				c[0] = PIXR(vid_buf[i]);
				c[1] = PIXG(vid_buf[i]);
				c[2] = PIXB(vid_buf[i]);
				fwrite(c,3,1,f);
			}
			vid_buf+=XRES+BARSIZE;
		}
		fclose(f);
		
		return 1;
	}
	
	return 0;
}
#else
int main(int argc, char *argv[])
{
	pixel *part_vbuf; //Extra video buffer
	pixel *part_vbuf_store;
	char uitext[512] = "";
	char heattext[256] = "";
	char coordtext[128] = "";
	int currentTime = 0;
	int FPS = 0, pastFPS = 0, elapsedTime = 0; 
	void *http_ver_check, *http_session_check = NULL;
	char *ver_data=NULL, *check_data=NULL, *tmp;
	//char console_error[255] = "";
	int result, i, j, bq, bc = 0, fire_fc=0, do_check=0, do_s_check=0, old_version=0, http_ret=0,http_s_ret=0, major, minor, buildnum, is_beta = 0, old_ver_len, new_message_len=0;
#ifdef INTERNAL
	int vs = 0;
#endif
	int wavelength_gfx = 0;
	int x, y, line_x, line_y, b = 0, sl=1, sr=0, su=0, c, lb = 0, lx = 0, ly = 0, lm = 0;//, tx, ty;
	int da = 0, dae = 0, db = 0, it = 2047, mx, my, bsx = 2, bsy = 2, quickoptions_tooltip_fade_invert, it_invert = 0;
	float nfvx, nfvy;
	int load_mode=0, load_w=0, load_h=0, load_x=0, load_y=0, load_size=0;
	void *load_data=NULL;
	pixel *load_img=NULL;//, *fbi_img=NULL;
	int save_mode=0, save_x=0, save_y=0, save_w=0, save_h=0, copy_mode=0;
	unsigned int rgbSave = PIXRGB(127,0,0);
	SDL_AudioSpec fmt;
	int username_flash = 0, username_flash_t = 1;
#ifdef PTW32_STATIC_LIB
    pthread_win32_process_attach_np();
    pthread_win32_thread_attach_np();
#endif
	limitFPS = 60;
	vid_buf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	part_vbuf = calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE); //Extra video buffer
	part_vbuf_store = part_vbuf;
	pers_bg = calloc((XRES+BARSIZE)*YRES, PIXELSIZE);

	//Allocate full size Gravmaps
	th_gravyf = calloc(XRES*YRES, sizeof(float));
	th_gravxf = calloc(XRES*YRES, sizeof(float));
	th_gravpf = calloc(XRES*YRES, sizeof(float));
	gravyf = calloc(XRES*YRES, sizeof(float));
	gravxf = calloc(XRES*YRES, sizeof(float));
	gravpf = calloc(XRES*YRES, sizeof(float));

	GSPEED = 1;

	/* Set 16-bit stereo audio at 22Khz */
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = 512;
	fmt.callback = mixaudio;
	fmt.userdata = NULL;

#ifdef LUACONSOLE
	luacon_open();
#endif
#ifdef MT
	numCores = core_count();
#endif
//TODO: Move out version stuff
	menu_count();
	parts = calloc(sizeof(particle), NPART);
	cb_parts = calloc(sizeof(particle), NPART);
	init_can_move();
	clear_sim();

	//fbi_img = render_packed_rgb(fbi, FBI_W, FBI_H, FBI_CMP);

	for (i=1; i<argc; i++)
	{
		if (!strncmp(argv[i], "ddir", 4) && i+1<argc)
		{
			chdir(argv[i+1]);
			i++;
		}
		else if (!strncmp(argv[i], "open", 4) && i+1<argc)
		{
			int size;
			void *file_data;
			file_data = file_load(argv[i+1], &size);
			if (file_data)
			{
				svf_last = file_data;
				svf_lsize = size;
				if(!parse_save(file_data, size, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap))
				{
					it=0;
					svf_filename[0] = 0;
					svf_fileopen = 1;
				} else {
					svf_last = NULL;
					svf_lsize = 0;
					free(file_data);
					file_data = NULL;
				}
			}
			i++;
		}

	}
	
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
			//sdl_scale = 2; //Removed because some displays cannot handle the resolution
			hud_enable = 0;
		}
		else if (!strncmp(argv[i], "sound", 5))
		{
			/* Open the audio device and start playing sound! */
			if ( SDL_OpenAudio(&fmt, NULL) < 0 )
			{
				fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
			}
			else
			{
				sound_enable = 1;
				SDL_PauseAudio(0);
			}
		}
		else if (!strncmp(argv[i], "scripts", 5))
		{
			file_script = 1;
		}
		else if (!strncmp(argv[i], "open", 4) && i+1<argc)
		{
			i++;
		}
		else if (!strncmp(argv[i], "ddir", 4) && i+1<argc)
		{
			i++;
		}

	}

	make_kernel();

	stamp_init();

	if (!sdl_open())
	{
		sdl_scale = 1;
		kiosk_enable = 0;
		if (!sdl_open()) exit(1);
	}
	save_presets(0);
	http_init(http_proxy_string[0] ? http_proxy_string : NULL);

	prepare_alpha(CELL, 1.0f);
	prepare_graphicscache();
	flm_data = generate_gradient(flm_data_colours, flm_data_pos, flm_data_points, 200);
	plasma_data = generate_gradient(plasma_data_colours, plasma_data_pos, plasma_data_points, 200);

	if (cpu_check())
	{
		error_ui(vid_buf, 0, "Unsupported CPU. Try another version.");
		return 1;
	}

	http_ver_check = http_async_req_start(NULL, "http://" SERVER "/Update.api?Action=CheckVersion", NULL, 0, 0);
	if (svf_login) {
		http_auth_headers(http_ver_check, svf_user_id, NULL, svf_session_id); //Add authentication so beta checking can be done from user basis
		http_session_check = http_async_req_start(NULL, "http://" SERVER "/Login.api?Action=CheckSession", NULL, 0, 0);
		http_auth_headers(http_session_check, svf_user_id, NULL, svf_session_id);
	}
#ifdef LUACONSOLE
	luacon_eval("dofile(\"autorun.lua\")"); //Autorun lua script
#endif
	while (!sdl_poll()) //the main loop
	{
		frameidx++;
		frameidx %= 30;
		if (!sys_pause||framerender) //only update air if not paused
		{
			update_air();
			if(aheat_enable)
				update_airh();
		}

		if(ngrav_enable && cmode==CM_FANCY)
		{
			part_vbuf = part_vbuf_store;
			memset(vid_buf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		} else {
			part_vbuf = vid_buf;
		}

		if(gravwl_timeout)
		{
			if(gravwl_timeout==1)
				gravity_mask();
			gravwl_timeout--;
		}
#ifdef OGLR
		if (cmode==CM_PERS)//save background for persistent, then clear
		{
			clearScreen(0.01f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
        }
		else //clear screen every frame
		{
            clearScreen(1.0f);
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
			if (cmode==CM_VEL || cmode==CM_PRESS || cmode==CM_CRACK || (cmode==CM_HEAT && aheat_enable))//air only gets drawn in these modes
			{
				draw_air(part_vbuf);
			}
		}
#else
		if (cmode==CM_VEL || cmode==CM_PRESS || cmode==CM_CRACK || (cmode==CM_HEAT && aheat_enable))//air only gets drawn in these modes
		{
			draw_air(part_vbuf);
		}
		else if (cmode==CM_PERS)//save background for persistent, then clear
		{
			memcpy(part_vbuf, pers_bg, (XRES+BARSIZE)*YRES*PIXELSIZE);
			memset(part_vbuf+((XRES+BARSIZE)*YRES), 0, ((XRES+BARSIZE)*YRES*PIXELSIZE)-((XRES+BARSIZE)*YRES*PIXELSIZE));
        }
		else //clear screen every frame
		{
			memset(part_vbuf, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
		}
#endif

		//Can't be too sure (Limit the cursor size)
		if (bsx>1180)
			bsx = 1180;
		if (bsx<0)
			bsx = 0;
		if (bsy>1180)
			bsy = 1180;
		if (bsy<0)
			bsy = 0;
			
		//Pretty powders, colour cycle
		//sandcolour_r = 0;
		//sandcolour_g = 0;
		sandcolour_b = sandcolour_r = sandcolour_g = (int)(20.0f*sin((float)sandcolour_frame*(M_PI/180.0f)));
		sandcolour_frame++;
		sandcolour_frame%=360;
		
		if(ngrav_enable && drawgrav_enable)
			draw_grav(vid_buf);
		draw_walls(part_vbuf);
		
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				debug_perf_time = ts.tv_nsec;
			#endif
		}
		
		update_particles(part_vbuf); //update everything
			
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				
				debug_perf_partitime[debug_perf_iend]  = ts.tv_nsec - debug_perf_time;
				
				debug_perf_time = ts.tv_nsec;
			#endif
		}
		
		render_parts(part_vbuf); //draw particles
		draw_other(part_vbuf);
		
		if(debug_flags & (DEBUG_PERFORMANCE_CALC|DEBUG_PERFORMANCE_FRAME))
		{
			#ifdef WIN32
			#elif defined(MACOSX)
			#else
				struct timespec ts;
				clock_gettime(CLOCK_REALTIME, &ts);
				
				debug_perf_frametime[debug_perf_iend]  = ts.tv_nsec - debug_perf_time;
			#endif
			debug_perf_iend++;
			debug_perf_iend %= DEBUG_PERF_FRAMECOUNT;
			debug_perf_istart++;
			debug_perf_istart %= DEBUG_PERF_FRAMECOUNT;
		}
		
		if(sl == WL_GRAV+100 || sr == WL_GRAV+100)
			draw_grav_zones(part_vbuf);
		
		if(ngrav_enable){
			pthread_mutex_lock(&gravmutex);
			result = grav_ready;
			if(result) //Did the gravity thread finish?
			{
				memcpy(th_gravmap, gravmap, sizeof(gravmap)); //Move our current gravmap to be processed other thread
				//memcpy(gravy, th_gravy, sizeof(gravy));	//Hmm, Gravy
				//memcpy(gravx, th_gravx, sizeof(gravx)); //Move the processed velocity maps to be used
				//memcpy(gravp, th_gravp, sizeof(gravp));

				if (!sys_pause||framerender){ //Only update if not paused
					//Switch the full size gravmaps, we don't really need the two above any more
					float *tmpf;
					tmpf = gravyf;
					gravyf = th_gravyf;
					th_gravyf = tmpf;

					tmpf = gravxf;
					gravxf = th_gravxf;
					th_gravxf = tmpf;

					tmpf = gravpf;
					gravpf = th_gravpf;
					th_gravpf = tmpf;

					grav_ready = 0; //Tell the other thread that we're ready for it to continue
					pthread_cond_signal(&gravcv);
				}
			}
			pthread_mutex_unlock(&gravmutex);
			//Apply the gravity mask
			membwand(gravy, gravmask, sizeof(gravy), sizeof(gravmask));
			membwand(gravx, gravmask, sizeof(gravx), sizeof(gravmask));
		}

		if (!sys_pause||framerender) //Only update if not paused
			memset(gravmap, 0, sizeof(gravmap)); //Clear the old gravmap

		if (framerender) {
			framerender = 0;
			sys_pause = 1;
		}

		if (cmode==CM_PERS)
		{
			if (!fire_fc)//fire_fc has nothing to do with fire... it is a counter for diminishing persistent view every 3 frames
			{
				dim_copy_pers(pers_bg, vid_buf);
			}
			else
			{
				memcpy(pers_bg, vid_buf, (XRES+BARSIZE)*YRES*PIXELSIZE);
			}
			fire_fc = (fire_fc+1) % 3;
		}
		
#ifndef OGLR
		if (cmode==CM_FIRE||cmode==CM_BLOB||cmode==CM_FANCY)
			render_fire(part_vbuf);
#endif

		render_signs(part_vbuf);

		if(ngrav_enable && cmode==CM_FANCY)
			render_gravlensing(part_vbuf, vid_buf);

		memset(vid_buf+((XRES+BARSIZE)*YRES), 0, (PIXELSIZE*(XRES+BARSIZE))*MENUSIZE);//clear menu areas
		clearrect(vid_buf, XRES-1, 0, BARSIZE+1, YRES);

		draw_svf_ui(vid_buf, sdl_mod & (KMOD_LCTRL|KMOD_RCTRL));
		
		if(debug_flags)
		{
			draw_debug_info(vid_buf, lm, lx, ly, x, y, line_x, line_y);
		}

		if (http_ver_check)
		{
			if (!do_check && http_async_req_status(http_ver_check))
			{
				ver_data = http_async_req_stop(http_ver_check, &http_ret, NULL);
				if (http_ret==200 && ver_data)
				{
					if (sscanf(ver_data, "%d.%d.%d.%d", &major, &minor, &is_beta, &buildnum)==4)
						if (buildnum>BUILD_NUM)
							old_version = 1;
						if (is_beta)
						{
							old_ver_len = textwidth((char*)old_ver_msg_beta);
						}
						else
						{
							old_ver_len = textwidth((char*)old_ver_msg);
						}
					free(ver_data);
				}
				http_ver_check = NULL;
			}
			do_check = (do_check+1) & 15;
		}
		if (http_session_check)
		{
			if (!do_s_check && http_async_req_status(http_session_check))
			{
				check_data = http_async_req_stop(http_session_check, &http_s_ret, NULL);
				if (http_ret==200 && check_data)
				{
					if (!strncmp(check_data, "EXPIRED", 7))
					{
						//Session expired
						printf("EXPIRED");
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
						svf_messages = 0;
					}
					else if (!strncmp(check_data, "BANNED", 6))
					{
						//User banned
						printf("BANNED");
						strcpy(svf_user, "");
						strcpy(svf_pass, "");
						strcpy(svf_user_id, "");
						strcpy(svf_session_id, "");
						svf_login = 0;
						svf_own = 0;
						svf_admin = 0;
						svf_mod = 0;
						svf_messages = 0;
						error_ui(vid_buf, 0, "Unable to log in\nYour account has been suspended, consider reading the rules.");
					}
					else if (!strncmp(check_data, "OK", 2))
					{
						//Session valid
						if (strlen(check_data)>2) {
							//User is elevated
							if (!strncmp(check_data+3, "ADMIN", 5))
							{
								//Check for messages
								svf_messages = atoi(check_data+9);
								svf_admin = 1;
								svf_mod = 0;
							}
							else if (!strncmp(check_data+3, "MOD", 3))
							{
								//Check for messages
								svf_messages = atoi(check_data+7);
								svf_admin = 0;
								svf_mod = 1;
							} else {
								//Check for messages
								svf_messages = atoi(check_data+3);
							}
						}
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
						svf_messages = 0;
					}
					save_presets(0);
					free(check_data);
				} else {
					//Unable to check session, YOU WILL BE TERMINATED
					strcpy(svf_user, "");
					strcpy(svf_pass, "");
					strcpy(svf_user_id, "");
					strcpy(svf_session_id, "");
					svf_login = 0;
					svf_own = 0;
					svf_admin = 0;
					svf_mod = 0;
					svf_messages = 0;
				}
				http_session_check = NULL;
			} else {
				clearrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14);
				drawrect(vid_buf, XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);
				drawtext(vid_buf, XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
				if (username_flash>30) {
					username_flash_t = -1;
					username_flash = 30;
				} else if (username_flash<0) {
					username_flash_t = 1;
					username_flash = 0;
				}
				username_flash += username_flash_t;
				if (svf_login)
					drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), svf_user, 255, 255, 255, 175-(username_flash*5));
				else
					drawtext(vid_buf, XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[checking]", 255, 255, 255, 255);
			}
			do_s_check = (do_s_check+1) & 15;
		}
#ifdef LUACONSOLE
	if(sdl_key){
		if(!luacon_keyevent(sdl_key, sdl_mod, LUACON_KDOWN))
			sdl_key = 0;
	}
	if(sdl_rkey){
		if(!luacon_keyevent(sdl_rkey, sdl_mod, LUACON_KUP))
			sdl_rkey = 0;
	}
#endif
		if (sys_shortcuts==1)//all shortcuts can be disabled by python scripts
		{
			if (sdl_key=='q' || sdl_key==SDLK_ESCAPE)
			{
				if (confirm_ui(vid_buf, "You are about to quit", "Are you sure you want to quit?", "Quit"))
				{
					break;
				}
			}
			if (sdl_key=='i' && (sdl_mod & KMOD_CTRL))
			{
				if(confirm_ui(vid_buf, "Install Powder Toy", "You are about to install The Powder Toy", "Install"))
				{
					if(register_extension())
					{
						info_ui(vid_buf, "Install success", "Powder Toy has been installed!");
					}
					else
					{
						error_ui(vid_buf, 0, "Install failed - You may not have permission or you may be on a platform that does not support installation");
					}
				}
			}
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
			if (sdl_key=='s' && ((sdl_mod & (KMOD_CTRL)) || !player2.spwn))
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
				if (sdl_zoom_trig)
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
				if (sdl_zoom_trig)
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
			if (sdl_key=='d' && ((sdl_mod & (KMOD_CTRL)) || !player2.spwn))
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
			if (sdl_key==SDLK_INSERT || sdl_key == SDLK_SEMICOLON)// || sdl_key==SDLK_BACKQUOTE)
				REPLACE_MODE = !REPLACE_MODE;
			if (sdl_key==SDLK_BACKQUOTE)
			{
				console_mode = !console_mode;
				//hud_enable = !console_mode;
			}
			if (sdl_key=='b')
			{
				if (sdl_mod & KMOD_CTRL)
				{
					decorations_enable = !decorations_enable;
					itc = 51;
					if (decorations_enable) strcpy(itc_msg, "Decorations layer: On");
					else strcpy(itc_msg, "Decorations layer: Off");
				}
				else
				{
					rgbSave = decorations_ui(vid_buf,&bsx,&bsy,rgbSave);//decoration_mode = !decoration_mode;
					decorations_enable = 1;
					sys_pause=1;
				}
			}
			if (sdl_key=='g')
			{
				if(sdl_mod & (KMOD_CTRL))
				{
					drawgrav_enable =! drawgrav_enable;
				}
				else
				{
					if (sdl_mod & (KMOD_SHIFT))
						GRID_MODE = (GRID_MODE+9)%10;
					else
						GRID_MODE = (GRID_MODE+1)%10;
				}
			}
			if (sdl_key=='m')
			{
				if(sl!=sr)
				{
					sl ^= sr;
					sr ^= sl;
					sl ^= sr;
				}
				dae = 51;
			}
			if (sdl_key=='=')
			{
				int nx, ny;
				if (sdl_mod & (KMOD_CTRL))
				{
					for (i=0; i<NPART; i++)
						if (parts[i].type==PT_SPRK)
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

			if (sdl_key=='w' && (!player2.spwn || (sdl_mod & (KMOD_SHIFT)))) //Gravity, by Moach
			{
				++gravityMode; // cycle gravity mode
				itc = 51;

				switch (gravityMode)
				{
				default:
					gravityMode = 0;
				case 0:
					strcpy(itc_msg, "Gravity: Vertical");
					break;
				case 1:
					strcpy(itc_msg, "Gravity: Off");
					break;
				case 2:
					strcpy(itc_msg, "Gravity: Radial");
					break;

				}
			}
			if (sdl_key=='y')
			{
				++airMode;
				itc = 52;

				switch (airMode)
				{
				default:
					airMode = 0;
				case 0:
					strcpy(itc_msg, "Air: On");
					break;
				case 1:
					strcpy(itc_msg, "Air: Pressure Off");
					break;
				case 2:
					strcpy(itc_msg, "Air: Velocity Off");
					break;
				case 3:
					strcpy(itc_msg, "Air: Off");
					break;
				case 4:
					strcpy(itc_msg, "Air: No Update");
					break;
				}
			}

			if (sdl_key=='t')
				VINE_MODE = !VINE_MODE;
			if (sdl_key==SDLK_SPACE)
				sys_pause = !sys_pause;
			if (sdl_key=='u')
				aheat_enable = !aheat_enable;
			if (sdl_key=='h' && !(sdl_mod & KMOD_LCTRL))
			{
				hud_enable = !hud_enable;
			}
			if (sdl_key==SDLK_F1 || (sdl_key=='h' && (sdl_mod & KMOD_LCTRL)))
			{
				if(!it)
				{
					it = 8047;
				}
				else
				{
					it = 0;
				}
			}
			if (sdl_key=='n')
				pretty_powder = !pretty_powder;
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
			if (load_mode==1)
			{
				matrix2d transform = m2d_identity;
				vector2d translate = v2d_zero;
				void *ndata;
				int doTransform = 0;
				if (sdl_key=='r'&&(sdl_mod & (KMOD_CTRL))&&(sdl_mod & (KMOD_SHIFT)))
				{
					transform = m2d_new(-1,0,0,1); //horizontal invert
					doTransform = 1;
				}
				else if (sdl_key=='r'&&(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))
				{
					transform = m2d_new(0,1,-1,0); //rotate anticlockwise 90 degrees
					doTransform = 1;
				}
				else if (sdl_mod & (KMOD_CTRL))
				{
					doTransform = 1;
					if (sdl_key==SDLK_LEFT) translate = v2d_new(-1,0);
					else if (sdl_key==SDLK_RIGHT) translate = v2d_new(1,0);
					else if (sdl_key==SDLK_UP) translate = v2d_new(0,-1);
					else if (sdl_key==SDLK_DOWN) translate = v2d_new(0,1);
					else doTransform = 0;
				}
				if (doTransform)
				{
					ndata = transform_save(load_data, &load_size, transform, translate);
					if (ndata!=load_data) free(load_data);
					free(load_img);
					load_data = ndata;
					load_img = prerender_save(load_data, load_size, &load_w, &load_h);
				}
			}
			if (sdl_key=='r'&&!(sdl_mod & (KMOD_CTRL|KMOD_SHIFT)))
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
				parts_lastActiveIndex = NPART-1;

				for (cby = 0; cby<YRES; cby++)
					for (cbx = 0; cbx<XRES; cbx++)
						pmap[cby][cbx] = cb_pmap[cby][cbx];

				for (cby = 0; cby<(YRES/CELL); cby++)
					for (cbx = 0; cbx<(XRES/CELL); cbx++)
					{
						vx[cby][cbx] = cb_vx[cby][cbx];
						vy[cby][cbx] = cb_vy[cby][cbx];
						pv[cby][cbx] = cb_pv[cby][cbx];
						hv[cby][cbx] = cb_hv[cby][cbx];
						bmap[cby][cbx] = cb_bmap[cby][cbx];
						emap[cby][cbx] = cb_emap[cby][cbx];
					}
			}
		}
#ifdef INTERNAL
		int counterthing;
		if (sdl_key=='v'&&!(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL)))//frame capture
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
			if (sdl_zoom_trig)//zoom window change
			{
				ZSIZE += sdl_wheel;
				if (ZSIZE>60)
					ZSIZE = 60;
				if (ZSIZE<2)
					ZSIZE = 2;
				ZFACTOR = 256/ZSIZE;
				sdl_wheel = 0;
			}
			else //change brush size
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

		bq = bc; // bq is previous mouse state
		bc = b = SDL_GetMouseState(&x, &y); // b is current mouse state

#ifdef LUACONSOLE
		if(bc && bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bc, LUACON_MPRESS)){
				b = 0;
			}
		}
		else if(bc && !bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bc, LUACON_MDOWN)){
				b = 0;
			}
		}
		else if(!bc && bq){
			if(!luacon_mouseevent(x/sdl_scale, y/sdl_scale, bq, LUACON_MUP)){
				b = 0;
			}
		}
		luacon_step(x/sdl_scale, y/sdl_scale,sl,sr);
#endif

		quickoptions_menu(vid_buf, b, bq, x, y);

		for (i=0; i<SC_TOTAL; i++)//draw all the menu sections
		{
			draw_menu(vid_buf, i, active_menu);
		}

		for (i=0; i<SC_TOTAL; i++)//check mouse position to see if it is on a menu section
		{
			if (!b&&x>=sdl_scale*(XRES-2) && x<sdl_scale*(XRES+BARSIZE-1) &&y>= sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)) && y<sdl_scale*((i*16)+YRES+MENUSIZE-16-(SC_TOTAL*16)+15))
			{
				active_menu = i;
			}
		}
		menu_ui_v3(vid_buf, active_menu, &sl, &sr, &dae, b, bq, x, y); //draw the elements in the current menu
		if (zoom_en && x>=sdl_scale*zoom_wx && y>=sdl_scale*zoom_wy //change mouse position while it is in a zoom window
		        && x<sdl_scale*(zoom_wx+ZFACTOR*ZSIZE)
		        && y<sdl_scale*(zoom_wy+ZFACTOR*ZSIZE))
		{
			x = (((x/sdl_scale-zoom_wx)/ZFACTOR)+zoom_x)*sdl_scale;
			y = (((y/sdl_scale-zoom_wy)/ZFACTOR)+zoom_y)*sdl_scale;
		}
		if (y>0 && y<sdl_scale*YRES && x>0 && x<sdl_scale*XRES)
		{
			int cr; //cr is particle under mouse, for drawing HUD information
			char nametext[50];
			if (photons[y/sdl_scale][x/sdl_scale]) {
				cr = photons[y/sdl_scale][x/sdl_scale];
			} else {
				cr = pmap[y/sdl_scale][x/sdl_scale];
			}
			if (cr)
			{
				if ((cr&0xFF)==PT_LIFE && parts[cr>>8].ctype>=0 && parts[cr>>8].ctype<NGOLALT)
				{
					sprintf(nametext, "%s (%s)", ptypes[cr&0xFF].name, gmenu[parts[cr>>8].ctype].name);
				}
				else if ((cr&0xFF)==PT_LAVA && parts[cr>>8].ctype > 0 && parts[cr>>8].ctype < PT_NUM )
				{
					char lowername[6];
					int ix;
					strcpy(lowername, ptypes[parts[cr>>8].ctype].name);
					for (ix = 0; lowername[ix]; ix++)
						lowername[ix] = tolower(lowername[ix]);

					sprintf(nametext, "Molten %s", lowername);
				}
				else if (DEBUG_MODE)
				{
					int tctype = parts[cr>>8].ctype;
					if ((cr&0xFF)==PT_PIPE)
					{
						tctype = parts[cr>>8].tmp&0xFF;
					}
					if (tctype>=PT_NUM || tctype<0 || (cr&0xFF)==PT_PHOT)
						tctype = 0;
					sprintf(nametext, "%s (%s)", ptypes[cr&0xFF].name, ptypes[tctype].name);
				}
				else
				{
					strcpy(nametext, ptypes[cr&0xFF].name);
				}
				if (DEBUG_MODE)
				{
					sprintf(heattext, "%s, Pressure: %3.2f, Temp: %4.2f C, Life: %d, Tmp:%d", nametext, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life, parts[cr>>8].tmp);
					sprintf(coordtext, "#%d, X:%d Y:%d", cr>>8, x/sdl_scale, y/sdl_scale);
				}
				else
				{
#ifdef BETA
					sprintf(heattext, "%s, Pressure: %3.2f, Temp: %4.2f C, Life: %d, Tmp:%d", nametext, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f, parts[cr>>8].life, parts[cr>>8].tmp);
#else
					sprintf(heattext, "%s, Pressure: %3.2f, Temp: %4.2f C", nametext, pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL], parts[cr>>8].temp-273.15f);
#endif
				}
				if ((cr&0xFF)==PT_PHOT) wavelength_gfx = parts[cr>>8].ctype;
			}
			else
			{
				sprintf(heattext, "Empty, Pressure: %3.2f", pv[(y/sdl_scale)/CELL][(x/sdl_scale)/CELL]);
				if (DEBUG_MODE)
				{
					sprintf(coordtext, "X:%d Y:%d. GX: %.2f GY: %.2f", x/sdl_scale, y/sdl_scale, gravxf[((y/sdl_scale)*XRES)+(x/sdl_scale)], gravyf[((y/sdl_scale)*XRES)+(x/sdl_scale)]);
				}
			}
		}


		mx = x;
		my = y;
		if (b && !bq && x>=(XRES-19-new_message_len)*sdl_scale &&
		        x<=(XRES-14)*sdl_scale && y>=(YRES-37)*sdl_scale && y<=(YRES-24)*sdl_scale && svf_messages)
		{
			open_link("http://" SERVER "/Conversations.html");
		}
		if (update_flag)
		{
			info_box(vid_buf, "Finalizing update...");
			if (last_build>BUILD_NUM)
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
			tmp = malloc(128);
#ifdef BETA
			if (is_beta)
			{
				sprintf(tmp, "Your version: %d.%d Beta (%d)\nNew version: %d.%d Beta (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
			else
			{
				sprintf(tmp, "Your version: %d.%d Beta (%d)\nNew version: %d.%d (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
#else
			if (is_beta)
			{
				sprintf(tmp, "Your version: %d.%d (%d)\nNew version: %d.%d Beta (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
			else
			{
				sprintf(tmp, "Your version: %d.%d (%d)\nNew version: %d.%d (%d)", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, major, minor, buildnum);
			}
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
		if (y>=sdl_scale*(YRES+(MENUSIZE-20))) //mouse checks for buttons at the bottom, to draw mouseover texts
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
			else if (x>=37*sdl_scale && x<=187*sdl_scale)
			{
				if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
				{
					db = 277;
					if (da < 51)
						da ++;
				}
				else if(svf_login)
				{
					db = 259;
					if (svf_open && svf_own && x<=55*sdl_scale)
						db = 258;
					if (da < 51)
						da ++;
				}
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
				if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
					db = 276;
				else
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
		else if (da > 0)//fade away mouseover text
			da --;

		if (dae > 0) //Fade away selected elements
			dae --;
		
		if (!sdl_zoom_trig && zoom_en==1)
			zoom_en = 0;

		if (sdl_key=='z' && zoom_en==2 && sys_shortcuts==1)
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
				parse_save(load_data, load_size, 0, load_x, load_y, bmap, fvx, fvy, signs, parts, pmap);
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
		else if (save_mode==1)//getting the area you are selecting
		{
			save_x = mx/sdl_scale;
			save_y = my/sdl_scale;
			if (save_x >= XRES) save_x = XRES-1;
			if (save_y >= YRES) save_y = YRES-1;
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
			save_w = mx/sdl_scale + 1 - save_x;
			save_h = my/sdl_scale + 1 - save_y;
			if (save_w+save_x>XRES) save_w = XRES-save_x;
			if (save_h+save_y>YRES) save_h = YRES-save_y;
			if (save_w<1) save_w = 1;
			if (save_h<1) save_h = 1;
			if (!b)
			{
				if (copy_mode==1)//CTRL-C, copy
				{
					clipboard_data=build_save(&clipboard_length, save_x, save_y, save_w, save_h, bmap, fvx, fvy, signs, parts);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
				}
				else if (copy_mode==2)//CTRL-X, cut
				{
					clipboard_data=build_save(&clipboard_length, save_x, save_y, save_w, save_h, bmap, fvx, fvy, signs, parts);
					clipboard_ready = 1;
					save_mode = 0;
					copy_mode = 0;
					clear_area(save_x, save_y, save_w, save_h);
				}
				else//normal save
				{
					stamp_save(save_x, save_y, save_w, save_h);
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
			{
				zoom_en = 2;
				sdl_zoom_trig = 0;
			}
		}
		else if (b)//there is a click
		{
			if (it > 50)
				it = 50;
			x /= sdl_scale;
			y /= sdl_scale;
			if (y>=YRES+(MENUSIZE-20))//check if mouse is on menu buttons
			{
				if (!lb)//mouse is NOT held down, so it is a first click
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
						//legacy_enable = !legacy_enable;
						simulation_ui(vid_buf);
					}
					if (x>=(XRES+BARSIZE-(510-367)) && x<=(XRES+BARSIZE-(510-383)) && !bq)
					{
						clear_sim();
						for (i=0; i<NPART-1; i++)
							parts[i].life = i+1;
						parts[NPART-1].life = -1;
						pfree = 0;

						legacy_enable = 0;
						svf_filename[0] = 0;
						svf_fileopen = 0;
						svf_myvote = 0;
						svf_open = 0;
						svf_publish = 0;
						svf_own = 0;
						svf_id[0] = 0;
						svf_name[0] = 0;
						svf_tags[0] = 0;
						svf_description[0] = 0;
						gravityMode = 0;
						airMode = 0;
					}
					if (x>=(XRES+BARSIZE-(510-385)) && x<=(XRES+BARSIZE-(510-476)))
					{
						login_ui(vid_buf);
						if (svf_login) {
							save_presets(0);
							http_session_check = NULL;
						}
					}
					if(sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
					{
						if (x>=37 && x<=187)
						{
							save_filename_ui(vid_buf);
									
						}
						if (x>=1 && x<=17)
						{
							catalogue_ui(vid_buf);
						}
					} else {
						if (x>=37 && x<=187 && svf_login)
						{
							if (!svf_open || !svf_own || x>51)
							{
								if (save_name_ui(vid_buf)) {
									execute_save(vid_buf);
									if (svf_id[0]) {
										copytext_ui(vid_buf, "Save ID", "Saved successfully!", svf_id);
									}
								}
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
							memset(pers_bg, 0, (XRES+BARSIZE)*YRES*PIXELSIZE);
							memset(fire_r, 0, sizeof(fire_r));
							memset(fire_g, 0, sizeof(fire_g));
							memset(fire_b, 0, sizeof(fire_b));
						}
					}
					if (x>=19 && x<=35 && svf_last && (svf_open || svf_fileopen) && !bq) {
						//int tpval = sys_pause;
						parse_save(svf_last, svf_lsize, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap);
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
					}
					if (x>=(XRES+BARSIZE-(510-494)) && x<=(XRES+BARSIZE-(510-509)) && !bq)
						sys_pause = !sys_pause;
					lb = 0;
				}
			}
			else if (y<YRES)// mouse is in playing field
			{
				int signi;

				c = (b&1) ? sl : sr; //c is element to be spawned
				su = c;

				if (c!=WL_SIGN+100 && c!=PT_FIGH)
				{
					if (!bq)
						for (signi=0; signi<MAXSIGNS; signi++)
							if (sregexp(signs[signi].text, "^{c:[0-9]*|.*}$")==0)
							{
								int signx, signy, signw, signh;
								get_sign_pos(signi, &signx, &signy, &signw, &signh);
								if (x>=signx && x<=signx+signw && y>=signy && y<=signy+signh)
								{
									char buff[256];
									int sldr;

									memset(buff, 0, sizeof(buff));

									for (sldr=3; signs[signi].text[sldr] != '|'; sldr++)
										buff[sldr-3] = signs[signi].text[sldr];

									buff[sldr-3] = '\0';
									open_ui(vid_buf, buff, 0);
								}
							}
				}

				if (c==WL_SIGN+100)
				{
					if (!bq)
						add_sign_ui(vid_buf, x, y);
				}

				if (c==PT_FIGH)
				{
					if (!bq)
						create_part(-1, x, y, PT_FIGH);
				}
				//for the click functions, lx and ly, are the positions of where the FIRST click happened.  x,y are current mouse position.
				else if (lb)//lb means you are holding mouse down
				{
					if (lm == 1)//line tool
					{
						if (sdl_mod & KMOD_ALT)
						{
							float snap_angle = floor(atan2(y-ly, x-lx)/(M_PI*0.25)+0.5)*M_PI*0.25;
							float line_mag = sqrtf(pow(x-lx,2)+pow(y-ly,2));
							line_x = (int)(line_mag*cos(snap_angle)+lx+0.5f);
							line_y = (int)(line_mag*sin(snap_angle)+ly+0.5f);
						}
						else
						{
							line_x = x;
							line_y = y;
						}
						xor_line(lx, ly, line_x, line_y, vid_buf);
						if (c==WL_FAN+100 && lx>=0 && ly>=0 && lx<XRES && ly<YRES && bmap[ly/CELL][lx/CELL]==WL_FAN)
						{
							nfvx = (line_x-lx)*0.005f;
							nfvy = (line_y-ly)*0.005f;
							flood_parts(lx, ly, WL_FANHELPER, -1, WL_FAN, 0);
							for (j=0; j<YRES/CELL; j++)
								for (i=0; i<XRES/CELL; i++)
									if (bmap[j][i] == WL_FANHELPER)
									{
										fvx[j][i] = nfvx;
										fvy[j][i] = nfvy;
										bmap[j][i] = WL_FAN;
									}
						}
						if (c == SPC_WIND)
						{
							for (j=-bsy; j<=bsy; j++)
								for (i=-bsx; i<=bsx; i++)
									if (lx+i>0 && ly+j>0 && lx+i<XRES && ly+j<YRES && InCurrentBrush(i,j,bsx,bsy))
									{
										vx[(ly+j)/CELL][(lx+i)/CELL] += (line_x-lx)*0.002f;
										vy[(ly+j)/CELL][(lx+i)/CELL] += (line_y-ly)*0.002f;
									}
						}
					}
					else if (lm == 2)//box tool
					{
						xor_line(lx, ly, lx, y, vid_buf);
						xor_line(lx, y, x, y, vid_buf);
						xor_line(x, y, x, ly, vid_buf);
						xor_line(x, ly, lx, ly, vid_buf);
					}
					else//while mouse is held down, it draws lines between previous and current positions
					{
						if (c == SPC_WIND)
						{
							for (j=-bsy; j<=bsy; j++)
								for (i=-bsx; i<=bsx; i++)
									if (x+i>0 && y+j>0 && x+i<XRES && y+j<YRES && InCurrentBrush(i,j,bsx,bsy))
									{
										vx[(y+j)/CELL][(x+i)/CELL] += (x-lx)*0.01f;
										vy[(y+j)/CELL][(x+i)/CELL] += (y-ly)*0.01f;
									}
						}
						else
						{
							create_line(lx, ly, x, y, bsx, bsy, c, get_brush_flags());
						}
						lx = x;
						ly = y;
					}
				}
				else //it is the first click
				{
					//start line tool
					if ((sdl_mod & (KMOD_SHIFT)) && !(sdl_mod & (KMOD_CTRL)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 1;//line
					}
					//start box tool
					else if ((sdl_mod & (KMOD_CTRL)) && !(sdl_mod & (KMOD_SHIFT|KMOD_ALT)))
					{
						lx = x;
						ly = y;
						lb = b;
						lm = 2;//box
					}
					//flood fill
					else if ((sdl_mod & (KMOD_CTRL)) && (sdl_mod & (KMOD_SHIFT)) && !(sdl_mod & (KMOD_ALT)))
					{
						if (sdl_mod & (KMOD_CAPS))
							c = 0;
						if (c!=WL_STREAM+100&&c!=SPC_AIR&&c!=SPC_HEAT&&c!=SPC_COOL&&c!=SPC_VACUUM&&!REPLACE_MODE&&c!=SPC_WIND&&c!=SPC_PGRV&&c!=SPC_NGRV)
							flood_parts(x, y, c, -1, -1, get_brush_flags());
						if (c==SPC_HEAT || c==SPC_COOL)
							create_parts(x, y, bsx, bsy, c, get_brush_flags());
						lx = x;
						ly = y;
						lb = 0;
						lm = 0;
					}
					//sample
					else if (((sdl_mod & (KMOD_ALT)) && !(sdl_mod & (KMOD_SHIFT|KMOD_CTRL))) || b==SDL_BUTTON_MIDDLE)
					{
						if (y>=0 && y<YRES && x>=0 && x<XRES)
						{
							int cr;
							cr = pmap[y][x];
							if (!cr)
								cr = photons[y][x];
							if (cr)
							{
								c = sl = cr&0xFF;
								if (c==PT_LIFE)
									c = sl = (parts[cr>>8].ctype << 8) | c;
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
					else //normal click, spawn element
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
								cb_hv[cby][cbx] = hv[cby][cbx];
								cb_bmap[cby][cbx] = bmap[cby][cbx];
								cb_emap[cby][cbx] = emap[cby][cbx];
							}
						create_parts(x, y, bsx, bsy, c, get_brush_flags());
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
			if (lb && lm) //lm is box/line tool
			{
				x /= sdl_scale;
				y /= sdl_scale;
				c = (lb&1) ? sl : sr;
				su = c;
				if (lm == 1)//line
				{
					if (c!=WL_FAN+100 || lx<0 || ly<0 || lx>=XRES || ly>=YRES || bmap[ly/CELL][lx/CELL]!=WL_FAN)
						create_line(lx, ly, line_x, line_y, bsx, bsy, c, get_brush_flags());
				}
				else//box
					create_box(lx, ly, x, y, c, get_brush_flags());
				lm = 0;
			}
			lb = 0;
		}

		if (load_mode)//draw preview of stamp
		{
			draw_image(vid_buf, load_img, load_x, load_y, load_w, load_h, 128);
			xor_rect(vid_buf, load_x, load_y, load_w, load_h);
		}

		if (save_mode)//draw dotted lines for selection
		{
			xor_rect(vid_buf, save_x, save_y, save_w, save_h);
			da = 51;//draws mouseover text for the message
			db = 269;//the save message
		}

		if (zoom_en!=1 && !load_mode && !save_mode)//draw normal cursor
		{
			render_cursor(vid_buf, mx/sdl_scale, my/sdl_scale, su, bsx, bsy);
			mousex = mx/sdl_scale;
			mousey = my/sdl_scale;
		}

		if (zoom_en)
			render_zoom(vid_buf);

		if (da)
			switch (db)//various mouseover messages, da is the alpha
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
				drawtext(vid_buf, 16, YRES-24, "Simulation options", 255, 255, 255, da*5);
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
			case 276:
				drawtext(vid_buf, 16, YRES-24, "Open a simulation from your hard drive.", 255, 255, 255, da*5);
				break;
			case 277:
				drawtext(vid_buf, 16, YRES-24, "Save the simulation to your hard drive.", 255, 255, 255, da*5);
				break;
			default:
				drawtext(vid_buf, 16, YRES-24, (char *)ptypes[db].descs, 255, 255, 255, da*5);
			}
		if (itc)//message in the middle of the screen, such as view mode changes
		{
			itc--;
			drawtext_outline(vid_buf, (XRES-textwidth(itc_msg))/2, ((YRES/2)-10), itc_msg, 255, 255, 255, itc>51?255:itc*5, 0, 0, 0, itc>51?255:itc*5);
		}
		if (it)//intro message
		{
			it--;
			drawtext(vid_buf, 16, 20, it_msg, 255, 255, 255, it>51?255:it*5);
		}

		if (old_version)
		{
			clearrect(vid_buf, XRES-21-old_ver_len, YRES-24, old_ver_len+9, 17);
			if (is_beta)
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg_beta, 255, 216, 32, 255);
			}
			else
			{
				drawtext(vid_buf, XRES-16-old_ver_len, YRES-19, old_ver_msg, 255, 216, 32, 255);
			}
			drawrect(vid_buf, XRES-19-old_ver_len, YRES-22, old_ver_len+5, 13, 255, 216, 32, 255);
		}
		
		if (svf_messages)
		{
			sprintf(new_message_msg, "You have %d new message%s, Click to view", svf_messages, (svf_messages>1)?"s":"");
			new_message_len = textwidth(new_message_msg);
			
			clearrect(vid_buf, XRES-21-new_message_len, YRES-39, new_message_len+9, 17);
			drawtext(vid_buf, XRES-16-new_message_len, YRES-34, new_message_msg, 255, 186, 32, 255);
			drawrect(vid_buf, XRES-19-new_message_len, YRES-37, new_message_len+5, 13, 255, 186, 32, 255);
		}

		FPS++;
		currentTime = SDL_GetTicks();
		elapsedTime = currentTime-pastFPS;
		if ((FPS>2 || elapsedTime>1000*2/limitFPS) && elapsedTime && FPS*1000/elapsedTime>limitFPS)
		{
			while (FPS*1000/elapsedTime>limitFPS)
			{
				SDL_Delay(1);
				currentTime = SDL_GetTicks();
				elapsedTime = currentTime-pastFPS;
			}
		}
		if (elapsedTime>=1000)
		{
			FPSB = FPS;
			FPS = 0;
			pastFPS = currentTime;
		}

		if (hud_enable)
		{
#ifdef BETA
			sprintf(uitext, "Version %d.%d Beta (%d) FPS:%d Parts:%d Generation:%d Gravity:%d Air:%d", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, FPSB, NUM_PARTS, GENERATION, gravityMode, airMode);
#else
			if (DEBUG_MODE)
				sprintf(uitext, "Version %d.%d (%d) FPS:%d Parts:%d Generation:%d Gravity:%d Air:%d", SAVE_VERSION, MINOR_VERSION, BUILD_NUM, FPSB, NUM_PARTS, GENERATION, gravityMode, airMode);
			else
				sprintf(uitext, "Version %d.%d FPS:%d", SAVE_VERSION, MINOR_VERSION, FPSB);
#endif
			if (REPLACE_MODE)
				strappend(uitext, " [REPLACE MODE]");
			if (sdl_mod&(KMOD_CAPS))
				strappend(uitext, " [CAP LOCKS]");
			if (GRID_MODE)
				sprintf(uitext, "%s [GRID: %d]", uitext, GRID_MODE); //TODO: Undefined behavior: variable is used as parameter and destination in sprintf().
#ifdef INTERNAL
			if (vs)
				strappend(uitext, " [FRAME CAPTURE]");
#endif
			quickoptions_tooltip_fade_invert = 255 - (quickoptions_tooltip_fade*20);
			it_invert = 50 - it;
			if(it_invert < 0)
				it_invert = 0;
			if(it_invert > 50)
				it_invert = 50;
			if (sdl_zoom_trig||zoom_en)
			{
				if (zoom_x<XRES/2)
				{
					fillrect(vid_buf, XRES-20-textwidth(heattext), 266, textwidth(heattext)+8, 15, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
					drawtext(vid_buf, XRES-16-textwidth(heattext), 270, heattext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
					if (DEBUG_MODE)
					{
						fillrect(vid_buf, XRES-20-textwidth(coordtext), 280, textwidth(coordtext)+8, 13, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
						drawtext(vid_buf, XRES-16-textwidth(coordtext), 282, coordtext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
					}
					if (wavelength_gfx)
						draw_wavelengths(vid_buf,XRES-20-textwidth(heattext),265,2,wavelength_gfx);
				}
				else
				{
					fillrect(vid_buf, 12, 266, textwidth(heattext)+8, 15, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
					drawtext(vid_buf, 16, 270, heattext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
					if (DEBUG_MODE)
					{
						fillrect(vid_buf, 12, 280, textwidth(coordtext)+8, 13, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
						drawtext(vid_buf, 16, 282, coordtext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
					}
					if (wavelength_gfx)
						draw_wavelengths(vid_buf,12,265,2,wavelength_gfx);
				}
			}
			else
			{
				fillrect(vid_buf, XRES-20-textwidth(heattext), 12, textwidth(heattext)+8, 15, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
				drawtext(vid_buf, XRES-16-textwidth(heattext), 16, heattext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
				if (DEBUG_MODE)
				{
					fillrect(vid_buf, XRES-20-textwidth(coordtext), 26, textwidth(coordtext)+8, 11, 0, 0, 0, quickoptions_tooltip_fade_invert*0.5);
					drawtext(vid_buf, XRES-16-textwidth(coordtext), 27, coordtext, 255, 255, 255, quickoptions_tooltip_fade_invert*0.75);
				}
				if (wavelength_gfx)
					draw_wavelengths(vid_buf,XRES-20-textwidth(heattext),11,2,wavelength_gfx);
			}
			wavelength_gfx = 0;
			drawtext_outline(vid_buf, 16, 16, uitext, 32, 216, 255, it_invert * 4, 0, 0, 0, it_invert * 4);

		}

		if (console_mode)
		{
#ifdef LUACONSOLE
			char *console;
			sys_pause = 1;
			console = console_ui(vid_buf, console_error, console_more);
			console = mystrdup(console);
			strcpy(console_error,"");
			if (process_command_lua(vid_buf, console, console_error)==-1)
			{
				free(console);
				break;
			}
			free(console);
			if (!console_mode)
				hud_enable = 1;
#else
			char *console;
			sys_pause = 1;
			console = console_ui(vid_buf, console_error, console_more);
			console = mystrdup(console);
			strcpy(console_error,"");
			if (process_command_old(vid_buf, console, console_error)==-1)
			{
				free(console);
				break;
			}
			free(console);
			if (!console_mode)
				hud_enable = 1;
#endif
		}

		//execute python step hook
		sdl_blit(0, 0, XRES+BARSIZE, YRES+MENUSIZE, vid_buf, XRES+BARSIZE);

		//Setting an element for the stick man
		if (player.spwn==0)
		{
			if ((sr<PT_NUM && ptypes[sr].falldown>0) || sr==SPC_AIR || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
				player.elem = sr;
			else
				player.elem = PT_DUST;
		}
		if (player2.spwn==0)
		{
			if ((sr<PT_NUM && ptypes[sr].falldown>0) || sr==SPC_AIR || sr == PT_NEUT || sr == PT_PHOT || sr == PT_LIGH)
				player2.elem = sr;
			else
				player2.elem = PT_DUST;
		}
	}
	SDL_CloseAudio();
	http_done();
#ifdef GRAVFFT
	grav_fft_cleanup();
#endif
#ifdef LUACONSOLE
	luacon_close();
#endif
#ifdef PTW32_STATIC_LIB
    pthread_win32_thread_detach_np();
    pthread_win32_process_detach_np();
#endif
	return 0;
}
#endif
