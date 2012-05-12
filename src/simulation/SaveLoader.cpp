/*
 * SaveLoader.cpp
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#include <bzlib.h>
#include <cmath>
#include "SaveLoader.h"

//!TODO: enum for LoadSave return

int SaveLoader::Info(unsigned char * data, int dataLength, int & width, int & height)
{
	unsigned char * saveData = data;
	if (dataLength<16)
	{
		return 1;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return OPSInfo(data, dataLength, width, height);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return PSVInfo(data, dataLength, width, height);
	}
	return 1;
}

int SaveLoader::Load(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x, int y)
{
	unsigned char * saveData = data;
	if (dataLength<16)
	{
		return 1;
	}
	if(saveData[0] == 'O' && saveData[1] == 'P' && saveData[2] == 'S')
	{
		return OPSLoad(data, dataLength, sim);
	}
	else if((saveData[0]==0x66 && saveData[1]==0x75 && saveData[2]==0x43) || (saveData[0]==0x50 && saveData[1]==0x53 && saveData[2]==0x76))
	{
		return PSVLoad(data, dataLength, sim, replace, x, y);
	}
	return 1;
}

unsigned char * SaveLoader::Build(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	unsigned char * temp = OPSBuild(dataLength, sim, orig_x0, orig_y0, orig_w, orig_h);
	if(!temp)
		temp = PSVBuild(dataLength, sim, orig_x0, orig_y0, orig_w, orig_h);
	return temp;
}

int SaveLoader::OPSInfo(unsigned char * data, int dataLength, int & width, int & height)
{
	return 2;
}


int SaveLoader::OPSLoad(unsigned char * data, int dataLength, Simulation * sim)
{
	return 2;
}

unsigned char * SaveLoader::OPSBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	return 0;
}

int SaveLoader::PSVInfo(unsigned char * data, int dataLength, int & width, int & height)
{
	width = data[6];
	height = data[7];
	return 0;
}

int SaveLoader::PSVLoad(unsigned char * data, int dataLength, Simulation * sim, bool replace, int x0, int y0)
{
	unsigned char * d = NULL, * c = data;
	int q,i,j,k,x,y,p=0,*m=NULL, ver, pty, ty, legacy_beta=0, tempGrav = 0;
	int bx0=x0/CELL, by0=y0/CELL, bw, bh, w, h;
	int nf=0, new_format = 0, ttv = 0;
	Particle *parts = sim->parts;
	int *fp = (int *)malloc(NPART*sizeof(int));

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	if (dataLength<16)
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
		sim->legacy_enable = 1;
	}
	else
	{
		if (ver>=44) {
			sim->legacy_enable = c[3]&0x01;
			if (!sim->sys_pause) {
				sim->sys_pause = (c[3]>>1)&0x01;
			}
			if (ver>=46 && replace) {
				sim->gravityMode = ((c[3]>>2)&0x03);// | ((c[3]>>2)&0x01);
				sim->airMode = ((c[3]>>4)&0x07);// | ((c[3]>>4)&0x02) | ((c[3]>>4)&0x01);
			}
			if (ver>=49 && replace) {
				tempGrav = ((c[3]>>7)&0x01);
			}
		} else {
			if (c[3]==1||c[3]==0) {
				sim->legacy_enable = c[3];
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
	d = (unsigned char *)malloc(i);
	if (!d)
		return 1;

	if (BZ2_bzBuffToBuffDecompress((char *)d, (unsigned *)&i, (char *)(c+12), dataLength-12, 0, 0))
		return 1;
	dataLength = i;

	if (dataLength < bw*bh)
		return 1;

	// normalize coordinates
	x0 = bx0*CELL;
	y0 = by0*CELL;
	w  = bw *CELL;
	h  = bh *CELL;

	if (replace)
	{
		if (ver<46) {
			sim->gravityMode = 0;
			sim->airMode = 0;
		}
		sim->clear_sim();
	}
	sim->parts_lastActiveIndex = NPART-1;
	m = (int *)calloc(XRES*YRES, sizeof(int));

	// make a catalog of free parts
	//memset(pmap, 0, sizeof(pmap)); "Using sizeof for array given as function argument returns the size of pointer."
	memset(sim->pmap, 0, sizeof(unsigned)*(XRES*YRES));
	for (i=0; i<NPART; i++)
		if (parts[i].type)
		{
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			sim->pmap[y][x] = (i<<8)|1;
		}
		else
			fp[nf++] = i;

	// load the required air state
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
		{
			if (d[p])
			{
				//In old saves, ignore walls created by sign tool bug
				//Not ignoring other invalid walls or invalid walls in new saves, so that any other bugs causing them are easier to notice, find and fix
				if (ver<71 && d[p]==O_WL_SIGN)
				{
					p++;
					continue;
				}

				sim->bmap[y][x] = d[p];
				if (sim->bmap[y][x]==1)
					sim->bmap[y][x]=WL_WALL;
				if (sim->bmap[y][x]==2)
					sim->bmap[y][x]=WL_DESTROYALL;
				if (sim->bmap[y][x]==3)
					sim->bmap[y][x]=WL_ALLOWLIQUID;
				if (sim->bmap[y][x]==4)
					sim->bmap[y][x]=WL_FAN;
				if (sim->bmap[y][x]==5)
					sim->bmap[y][x]=WL_STREAM;
				if (sim->bmap[y][x]==6)
					sim->bmap[y][x]=WL_DETECT;
				if (sim->bmap[y][x]==7)
					sim->bmap[y][x]=WL_EWALL;
				if (sim->bmap[y][x]==8)
					sim->bmap[y][x]=WL_WALLELEC;
				if (sim->bmap[y][x]==9)
					sim->bmap[y][x]=WL_ALLOWAIR;
				if (sim->bmap[y][x]==10)
					sim->bmap[y][x]=WL_ALLOWSOLID;
				if (sim->bmap[y][x]==11)
					sim->bmap[y][x]=WL_ALLOWALLELEC;
				if (sim->bmap[y][x]==12)
					sim->bmap[y][x]=WL_EHOLE;
				if (sim->bmap[y][x]==13)
					sim->bmap[y][x]=WL_ALLOWGAS;
				
				if (sim->bmap[y][x]==O_WL_WALLELEC)
					sim->bmap[y][x]=WL_WALLELEC;
				if (sim->bmap[y][x]==O_WL_EWALL)
					sim->bmap[y][x]=WL_EWALL;
				if (sim->bmap[y][x]==O_WL_DETECT)
					sim->bmap[y][x]=WL_DETECT;
				if (sim->bmap[y][x]==O_WL_STREAM)
					sim->bmap[y][x]=WL_STREAM;
				if (sim->bmap[y][x]==O_WL_FAN||sim->bmap[y][x]==O_WL_FANHELPER)
					sim->bmap[y][x]=WL_FAN;
				if (sim->bmap[y][x]==O_WL_ALLOWLIQUID)
					sim->bmap[y][x]=WL_ALLOWLIQUID;
				if (sim->bmap[y][x]==O_WL_DESTROYALL)
					sim->bmap[y][x]=WL_DESTROYALL;
				if (sim->bmap[y][x]==O_WL_ERASE)
					sim->bmap[y][x]=WL_ERASE;
				if (sim->bmap[y][x]==O_WL_WALL)
					sim->bmap[y][x]=WL_WALL;
				if (sim->bmap[y][x]==O_WL_ALLOWAIR)
					sim->bmap[y][x]=WL_ALLOWAIR;
				if (sim->bmap[y][x]==O_WL_ALLOWSOLID)
					sim->bmap[y][x]=WL_ALLOWSOLID;
				if (sim->bmap[y][x]==O_WL_ALLOWALLELEC)
					sim->bmap[y][x]=WL_ALLOWALLELEC;
				if (sim->bmap[y][x]==O_WL_EHOLE)
					sim->bmap[y][x]=WL_EHOLE;
				if (sim->bmap[y][x]==O_WL_ALLOWGAS)
					sim->bmap[y][x]=WL_ALLOWGAS;
				if (sim->bmap[y][x]==O_WL_GRAV)
					sim->bmap[y][x]=WL_GRAV;
				if (sim->bmap[y][x]==O_WL_ALLOWENERGY)
					sim->bmap[y][x]=WL_ALLOWENERGY;
			}

			p++;
		}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				sim->fvx[y][x] = (d[p++]-127.0f)/64.0f;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (d[(y-by0)*bw+(x-bx0)]==4||d[(y-by0)*bw+(x-bx0)]==WL_FAN)
			{
				if (p >= dataLength)
					goto corrupt;
				sim->fvy[y][x] = (d[p++]-127.0f)/64.0f;
			}

	// load the particle map
	i = 0;
	pty = p;
	for (y=y0; y<y0+h; y++)
		for (x=x0; x<x0+w; x++)
		{
			if (p >= dataLength)
				goto corrupt;
			j=d[p++];
			if (j >= PT_NUM) {
				//TODO: Possibly some server side translation
				j = PT_DUST;//goto corrupt;
			}
			sim->gol[x][y]=0;
			if (j)
			{
				if (sim->pmap[y][x])
				{
					k = sim->pmap[y][x]>>8;
				}
				else if (i<nf)
				{
					k = fp[i];
					i++;
				}
				else
				{
					m[(x-x0)+(y-y0)*w] = NPART+1;
					continue;
				}
				memset(parts+k, 0, sizeof(Particle));
				parts[k].type = j;
				if (j == PT_COAL)
					parts[k].tmp = 50;
				if (j == PT_FUSE)
					parts[k].tmp = 50;
				if (j == PT_PHOT)
					parts[k].ctype = 0x3fffffff;
				if (j == PT_SOAP)
					parts[k].ctype = 0;
				if (j==PT_BIZR || j==PT_BIZRG || j==PT_BIZRS)
					parts[k].ctype = 0x47FFFF;
				parts[k].x = (float)x;
				parts[k].y = (float)y;
				m[(x-x0)+(y-y0)*w] = k+1;
			}
		}

	// load particle properties
	for (j=0; j<w*h; j++)
	{
		i = m[j];
		if (i)
		{
			i--;
			if (p+1 >= dataLength)
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
				if (p >= dataLength) {
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
				if (p >= dataLength)
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
				if (p >= dataLength) {
					goto corrupt;
				}
				if (i <= NPART) {
					ttv = (d[p++])<<8;
					ttv |= (d[p++]);
					parts[i-1].tmp = ttv;
					if (ver<53 && !parts[i-1].tmp)
						for (q = 1; q<=NGOLALT; q++) {
							if (parts[i-1].type==sim->goltype[q-1] && sim->grule[q][9]==2)
								parts[i-1].tmp = sim->grule[q][9]-1;
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
				if (p >= dataLength)
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
				if (p >= dataLength) {
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
				if (p >= dataLength) {
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
				if (p >= dataLength) {
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
				if (p >= dataLength) {
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
				if (p >= dataLength)
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
				parts[i-1].temp = sim->elements[parts[i-1].type].Temperature;
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
			if (p >= dataLength)
				goto corrupt;
			if (i <= NPART)
				parts[i-1].ctype = d[p++];
			else
				p++;
		}
		//TODO: STKM_init_legs
		// no more particle properties to load, so we can change type here without messing up loading
		if (i && i<=NPART)
		{
			if ((sim->player.spwn == 1 && ty==PT_STKM) || (sim->player2.spwn == 1 && ty==PT_STKM2))
			{
				parts[i-1].type = PT_NONE;
			}
			else if (parts[i-1].type == PT_STKM)
			{
				//STKM_init_legs(&player, i-1);
				sim->player.spwn = 1;
				sim->player.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_STKM2)
			{
				//STKM_init_legs(&player2, i-1);
				sim->player2.spwn = 1;
				sim->player2.elem = PT_DUST;
			}
			else if (parts[i-1].type == PT_FIGH)
			{
				unsigned char fcount = 0;
				while (fcount < 100 && fcount < (sim->fighcount+1) && sim->fighters[fcount].spwn==1) fcount++;
				if (fcount < 100 && sim->fighters[fcount].spwn==0)
				{
					parts[i-1].tmp = fcount;
					sim->fighters[fcount].spwn = 1;
					sim->fighters[fcount].elem = PT_DUST;
					sim->fighcount++;
					//STKM_init_legs(&(fighters[fcount]), i-1);
				}
			}
			else if (parts[i-1].type == PT_SPNG)
			{
				if (fabs(parts[i-1].vx)>0.0f || fabs(parts[i-1].vy)>0.0f)
					parts[i-1].flags |= FLAG_MOVABLE;
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
					if (ty==sim->goltype[gnum])
						parts[i-1].ctype = gnum;
				}
				ty = PT_LIFE;
			}
			if(ver<52 && (ty==PT_CLNE || ty==PT_PCLN || ty==PT_BCLN)){
				//Replace old GOL ctypes in clone
				for (gnum = 0; gnum<NGOLALT; gnum++){
					if (parts[i-1].ctype==sim->goltype[gnum])
					{
						parts[i-1].ctype = PT_LIFE;
						parts[i-1].tmp = gnum;
					}
				}
			}
			if(ty==PT_LCRY){
				if(ver<67)
				{
					//New LCRY uses TMP not life
					if(parts[i-1].life>=10)
					{
						parts[i-1].life = 10;
						parts[i-1].tmp2 = 10;
						parts[i-1].tmp = 3;
					}
					else if(parts[i-1].life<=0)
					{
						parts[i-1].life = 0;
						parts[i-1].tmp2 = 0;
						parts[i-1].tmp = 0;
					}
					else if(parts[i-1].life < 10 && parts[i-1].life > 0)
					{
						parts[i-1].tmp = 1;
					}
				}
				else
				{
					parts[i-1].tmp2 = parts[i-1].life;
				}
			}
			if (!sim->elements[parts[i-1].type].Enabled)
				parts[i-1].type = PT_NONE;
		}
	}

	#ifndef RENDERER
	//Change the gravity state
	if(sim->ngrav_enable != tempGrav && replace)
	{
		if(tempGrav)
			sim->grav->start_grav_async();
		else
			sim->grav->stop_grav_async();
	}
	#endif

	sim->grav->gravity_mask();

	if (p >= dataLength)
		goto version1;
	j = d[p++];
	for (i=0; i<j; i++)
	{
		if (p+6 > dataLength)
			goto corrupt;
		for (k=0; k<MAXSIGNS; k++)
			if (!sim->signs[k].text[0])
				break;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			sim->signs[k].x = x+x0;
		x = d[p++];
		x |= ((unsigned)d[p++])<<8;
		if (k<MAXSIGNS)
			sim->signs[k].y = x+y0;
		x = d[p++];
		if (k<MAXSIGNS)
			sim->signs[k].ju = x;
		x = d[p++];
		if (p+x > dataLength)
			goto corrupt;
		if (k<MAXSIGNS)
		{
			memcpy(sim->signs[k].text, d+p, x);
			sim->signs[k].text[x] = 0;
			//clean_text(signs[k].text, 158-14 /* Current max sign length */); //TODO: Text cleanup for signs
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
		sim->legacy_enable = 0;
		sim->clear_sim();
	}
	return 1;
}

unsigned char * SaveLoader::PSVBuild(int & dataLength, Simulation * sim, int orig_x0, int orig_y0, int orig_w, int orig_h)
{
	unsigned char *d = (unsigned char*)calloc(1,3*(XRES/CELL)*(YRES/CELL)+(XRES*YRES)*15+MAXSIGNS*262), *c;
	int i,j,x,y,p=0,*m=(int*)calloc(XRES*YRES, sizeof(int));
	int x0, y0, w, h, bx0=orig_x0/CELL, by0=orig_y0/CELL, bw, bh;
	Particle *parts = sim->parts;
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
			d[p++] = sim->bmap[y][x];
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (sim->bmap[y][x]==WL_FAN||sim->bmap[y][x]==4)
			{
				i = (int)(sim->fvx[y][x]*64.0f+127.5f);
				if (i<0) i=0;
				if (i>255) i=255;
				d[p++] = i;
			}
	for (y=by0; y<by0+bh; y++)
		for (x=bx0; x<bx0+bw; x++)
			if (sim->bmap[y][x]==WL_FAN||sim->bmap[y][x]==4)
			{
				i = (int)(sim->fvy[y][x]*64.0f+127.5f);
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
		if (sim->signs[i].text[0] &&
				sim->signs[i].x>=x0 && sim->signs[i].x<x0+w &&
				sim->signs[i].y>=y0 && sim->signs[i].y<y0+h)
			j++;
	d[p++] = j;
	for (i=0; i<MAXSIGNS; i++)
		if (sim->signs[i].text[0] &&
				sim->signs[i].x>=x0 && sim->signs[i].x<x0+w &&
				sim->signs[i].y>=y0 && sim->signs[i].y<y0+h)
		{
			d[p++] = (sim->signs[i].x-x0);
			d[p++] = (sim->signs[i].x-x0)>>8;
			d[p++] = (sim->signs[i].y-y0);
			d[p++] = (sim->signs[i].y-y0)>>8;
			d[p++] = sim->signs[i].ju;
			x = strlen(sim->signs[i].text);
			d[p++] = x;
			memcpy(d+p, sim->signs[i].text, x);
			p+=x;
		}

	i = (p*101+99)/100 + 612;
	c = (unsigned char*)malloc(i);

	//New file header uses PSv, replacing fuC. This is to detect if the client uses a new save format for temperatures
	//This creates a problem for old clients, that display and "corrupt" error instead of a "newer version" error

	c[0] = 0x50;	//0x66;
	c[1] = 0x53;	//0x75;
	c[2] = 0x76;	//0x43;
	c[3] = sim->legacy_enable|((sim->sys_pause<<1)&0x02)|((sim->gravityMode<<2)&0x0C)|((sim->airMode<<4)&0x70)|((sim->ngrav_enable<<7)&0x80);
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

	dataLength = i+12;
	return c;
}
