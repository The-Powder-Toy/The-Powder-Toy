#include <stdint.h>
#include <math.h>
#include <defines.h>
#include <powder.h>
#include <air.h>
#include <misc.h>

int isplayer = 0;
float player[27]; //[0] is a command cell, [3]-[18] are legs positions, [19] is index, [19]-[26] are accelerations
float player2[27];

particle *parts;
particle *cb_parts;

int gravityMode = 1; // starts enabled in "vertical" mode...

unsigned char bmap[YRES/CELL][XRES/CELL];
unsigned char emap[YRES/CELL][XRES/CELL];

unsigned char cb_bmap[YRES/CELL][XRES/CELL];
unsigned char cb_emap[YRES/CELL][XRES/CELL];

int pfree;

unsigned pmap[YRES][XRES];
unsigned cb_pmap[YRES][XRES];
unsigned photons[YRES][XRES];

static int pn_junction_sprk(int x, int y, int pt)
{
	unsigned r = pmap[y][x];
	if ((r & 0xFF) != pt)
		return 0;
	r >>= 8;
	if (parts[r].type != pt)
		return 0;
	if (parts[r].life != 0)
		return 0;

	parts[r].ctype = pt;
	part_change_type(r,x,y,PT_SPRK);
	parts[r].life = 4;
	return 1;
}

static void photoelectric_effect(int nx, int ny)
{
	unsigned r = pmap[ny][nx];

	if ((r&0xFF) == PT_PSCN) {
		if ((pmap[ny][nx-1] & 0xFF) == PT_NSCN ||
		        (pmap[ny][nx+1] & 0xFF) == PT_NSCN ||
		        (pmap[ny-1][nx] & 0xFF) == PT_NSCN ||
		        (pmap[ny+1][nx] & 0xFF) == PT_NSCN)
			pn_junction_sprk(nx, ny, PT_PSCN);
	}
}
/*
   RETURN-value explenation
1 = Swap
0 = No move/Bounce
2 = Both particles occupy the same space.
 */
int eval_move(int pt, int nx, int ny, unsigned *rr)
{
	unsigned r;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r && (r>>8)<NPART)
		r = (r&~0xFF) | parts[r>>8].type;
	if (rr)
		*rr = r;

	if ((r&0xFF)==PT_VOID || (r&0xFF)==PT_BHOL)
		return 1;

	if (pt==PT_PHOT&&(
	            (r&0xFF)==PT_GLAS || (r&0xFF)==PT_PHOT ||
	            (r&0xFF)==PT_CLNE || (r&0xFF)==PT_PCLN ||
	            (r&0xFF)==PT_GLOW || (r&0xFF)==PT_WATR ||
	            (r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW ||
	            (r&0xFF)==PT_ISOZ || (r&0xFF)==PT_ISZS ||
	            (r&0xFF)==PT_FILT || (r&0xFF)==PT_INVIS ||
	            (r&0xFF)==PT_QRTZ || (r&0xFF)==PT_PQRT ||
	            ((r&0xFF)==PT_LCRY&&parts[r>>8].life > 5)))
		return 2;

	if (pt==PT_STKM) //Stick man's head shouldn't collide
		return 2;
	if (pt==PT_STKM2) //Stick man's head shouldn't collide
		return 2;
	if ((pt==PT_BIZR||pt==PT_BIZRG)&&(r&0xFF)==PT_FILT)
		return 2;
	if (bmap[ny/CELL][nx/CELL]==WL_ALLOWGAS && ptypes[pt].falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
		return 0;
	if (ptypes[pt].falldown!=2 && bmap[ny/CELL][nx/CELL]==WL_ALLOWLIQUID)
		return 0;
	if ((pt==PT_NEUT ||pt==PT_PHOT) && bmap[ny/CELL][nx/CELL]==WL_EWALL && !emap[ny/CELL][nx/CELL])
		return 0;
	if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL])
		return 2;
	if (bmap[ny/CELL][nx/CELL]==WL_ALLOWAIR)
		return 0;

	if (ptypes[pt].falldown!=1 && bmap[ny/CELL][nx/CELL]==WL_ALLOWSOLID)
		return 0;
	if (r && (r&0xFF) < PT_NUM) {
		if (ptypes[pt].properties&TYPE_ENERGY && ptypes[(r&0xFF)].properties&TYPE_ENERGY)
			return 2;

		if (pt==PT_NEUT && ptypes[(r&0xFF)].properties&PROP_NEUTPASS)
			return 2;
		if (pt==PT_NEUT && ptypes[(r&0xFF)].properties&PROP_NEUTPENETRATE)
			return 1;
		if ((r&0xFF)==PT_NEUT && ptypes[pt].properties&PROP_NEUTPENETRATE)
			return 0;
	}

	if (r && ((r&0xFF) >= PT_NUM || (ptypes[pt].weight <= ptypes[(r&0xFF)].weight)))
		return 0;

	if (pt == PT_PHOT)
		return 2;

	return 1;
}

int try_move(int i, int x, int y, int nx, int ny)
{
	unsigned r, e;

	if (x==nx && y==ny)
		return 1;

	e = eval_move(parts[i].type, nx, ny, &r);

	if ((pmap[ny][nx]&0xFF)==PT_BOMB && parts[i].type==PT_BOMB && parts[i].tmp == 1)
		e = 2;

	if ((pmap[ny][nx]&0xFF)==PT_INVIS && (pv[ny/CELL][nx/CELL]>4.0f ||pv[ny/CELL][nx/CELL]<-4.0f))
		return 1;
	/* half-silvered mirror */
	if (!e && parts[i].type==PT_PHOT &&
	        (((r&0xFF)==PT_BMTL && rand()<RAND_MAX/2) ||
	         (pmap[y][x]&0xFF)==PT_BMTL))
		e = 2;

	if (!e)
	{
		if (!legacy_enable && parts[i].type==PT_PHOT && r)
		{
			if ((r & 0xFF) == PT_COAL || (r & 0xFF) == PT_BCOL)
				parts[r>>8].temp = parts[i].temp;

			if ((r & 0xFF) < PT_NUM && ptypes[r&0xFF].hconduct)
				parts[i].temp = parts[r>>8].temp = restrict_flt((parts[r>>8].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}
		return 0;
	}

	if (e == 2)
	{
		if (parts[i].type == PT_PHOT && (r&0xFF)==PT_GLOW && !parts[r>>8].life)
			if (rand() < RAND_MAX/30)
			{
				parts[r>>8].life = 120;
				create_gain_photon(i);
			}
		if (parts[i].type == PT_PHOT && (r&0xFF)==PT_FILT)
		{
			int temp_bin = (int)((parts[r>>8].temp-273.0f)*0.025f);
			if (temp_bin < 0) temp_bin = 0;
			if (temp_bin > 25) temp_bin = 25;
			parts[i].ctype = 0x1F << temp_bin;
		}
		if (parts[i].type == PT_NEUT && (r&0xFF)==PT_GLAS) {
			if (rand() < RAND_MAX/10)
				create_cherenkov_photon(i);
		}
		if (parts[i].type == PT_PHOT && (r&0xFF)==PT_INVIS) {
			part_change_type(i,x,y,PT_NEUT);
			parts[i].ctype = 0;
		}
		if ((parts[i].type==PT_BIZR||parts[i].type==PT_BIZRG) && (r&0xFF)==PT_FILT)
		{
			int temp_bin = (int)((parts[r>>8].temp-273.0f)*0.025f);
			if (temp_bin < 0) temp_bin = 0;
			if (temp_bin > 25) temp_bin = 25;
			parts[i].ctype = 0x1F << temp_bin;
		}
		return 1;
	}

	if ((r&0xFF)==PT_VOID)
	{
		if (parts[i].type == PT_STKM)
		{
			death = 1;
			isplayer = 0;
		}
		if (parts[i].type == PT_STKM2)
		{
			death2 = 1;
			isplayer2 = 0;
		}
		parts[i].type=PT_NONE;
		return 0;
	}
	if ((r&0xFF)==PT_BHOL)
	{
		if (parts[i].type == PT_STKM)
		{
			death = 1;
			isplayer = 0;
		}
		if (parts[i].type == PT_STKM2)
		{
			death2 = 1;
			isplayer2 = 0;
		}
		parts[i].type=PT_NONE;
		if (!legacy_enable)
		{
			parts[r>>8].temp = restrict_flt(parts[r>>8].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);//3.0f;
		}

		return 0;
	}
	if ((pmap[ny][nx]&0xFF)==PT_CNCT)
		return 0;
	if (parts[i].type==PT_CNCT && y<ny && (pmap[y+1][x]&0xFF)==PT_CNCT)
		return 0;

	if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL])
		return 1;
	if ((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && (bmap[ny/CELL][nx/CELL]!=WL_EHOLE && !emap[ny/CELL][nx/CELL]))
		return 0;

	if (r && (r>>8)<NPART && ptypes[r&0xFF].falldown!=2 && bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID)
		return 0;

	if (parts[i].type == PT_PHOT)
		return 1;

	e = r >> 8;
	if (r && e<NPART)
	{
		if (parts[e].type == PT_PHOT)
			return 1;

		parts[e].x += x-nx;
		parts[e].y += y-ny;
	}

	pmap[ny][nx] = (i<<8)|parts[i].type;
	pmap[y][x] = r;

	return 1;
}

static unsigned direction_to_map(float dx, float dy, int t)
{
	// TODO:
	// Adding extra directions causes some inaccuracies.
	// Not adding them causes problems with some diagonal surfaces (photons absorbed instead of reflected).
	// For now, don't add them.
	// Solution may involve more intelligent setting of initial i0 value in find_next_boundary?
	// or rewriting normal/boundary finding code

	return (dx >= 0) |
	       (((dx + dy) >= 0) << 1) |     /*  567  */
	       ((dy >= 0) << 2) |            /*  4+0  */
	       (((dy - dx) >= 0) << 3) |     /*  321  */
	       ((dx <= 0) << 4) |
	       (((dx + dy) <= 0) << 5) |
	       ((dy <= 0) << 6) |
	       (((dy - dx) <= 0) << 7);
	/*
	return (dx >= -0.001) |
	       (((dx + dy) >= -0.001) << 1) |     //  567
	       ((dy >= -0.001) << 2) |            //  4+0
	       (((dy - dx) >= -0.001) << 3) |     //  321
	       ((dx <= 0.001) << 4) |
	       (((dx + dy) <= 0.001) << 5) |
	       ((dy <= 0.001) << 6) |
	       (((dy - dx) <= 0.001) << 7);
	}*/
}

static int is_blocking(int t, int x, int y)
{
	if (t & REFRACT) {
		if (x<0 || y<0 || x>=XRES || y>=YRES)
			return 0;
		if ((pmap[y][x] & 0xFF) == PT_GLAS)
			return 1;
		return 0;
	}

	return !eval_move(t, x, y, NULL);
}

static int is_boundary(int pt, int x, int y)
{
	if (!is_blocking(pt,x,y))
		return 0;
	if (is_blocking(pt,x,y-1) && is_blocking(pt,x,y+1) && is_blocking(pt,x-1,y) && is_blocking(pt,x+1,y))
		return 0;
	return 1;
}

static int find_next_boundary(int pt, int *x, int *y, int dm, int *em)
{
	static int dx[8] = {1,1,0,-1,-1,-1,0,1};
	static int dy[8] = {0,1,1,1,0,-1,-1,-1};
	static int de[8] = {0x83,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC1};
	int i, ii, i0;

	if (*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
		return 0;

	if (*em != -1) {
		i0 = *em;
		dm &= de[i0];
	} else
		i0 = 0;

	for (ii=0; ii<8; ii++) {
		i = (ii + i0) & 7;
		if ((dm & (1 << i)) && is_boundary(pt, *x+dx[i], *y+dy[i])) {
			*x += dx[i];
			*y += dy[i];
			*em = i;
			return 1;
		}
	}

	return 0;
}

int get_normal(int pt, int x, int y, float dx, float dy, float *nx, float *ny)
{
	int ldm, rdm, lm, rm;
	int lx, ly, lv, rx, ry, rv;
	int i, j;
	float r, ex, ey;

	if (!dx && !dy)
		return 0;

	if (!is_boundary(pt, x, y))
		return 0;

	ldm = direction_to_map(-dy, dx, pt);
	rdm = direction_to_map(dy, -dx, pt);
	lx = rx = x;
	ly = ry = y;
	lv = rv = 1;
	lm = rm = -1;

	j = 0;
	for (i=0; i<SURF_RANGE; i++) {
		if (lv)
			lv = find_next_boundary(pt, &lx, &ly, ldm, &lm);
		if (rv)
			rv = find_next_boundary(pt, &rx, &ry, rdm, &rm);
		j += lv + rv;
		if (!lv && !rv)
			break;
	}

	if (j < NORMAL_MIN_EST)
		return 0;

	if ((lx == rx) && (ly == ry))
		return 0;

	ex = rx - lx;
	ey = ry - ly;
	r = 1.0f/hypot(ex, ey);
	*nx =  ey * r;
	*ny = -ex * r;

	return 1;
}

int get_normal_interp(int pt, float x0, float y0, float dx, float dy, float *nx, float *ny)
{
	int x, y, i;

	dx /= NORMAL_FRAC;
	dy /= NORMAL_FRAC;

	for (i=0; i<NORMAL_INTERP; i++) {
		x = (int)(x0 + 0.5f);
		y = (int)(y0 + 0.5f);
		if (is_boundary(pt, x, y))
			break;
		x0 += dx;
		y0 += dy;
	}
	if (i >= NORMAL_INTERP)
		return 0;

	if (pt == PT_PHOT)
		photoelectric_effect(x, y);

	return get_normal(pt, x, y, dx, dy, nx, ny);
}

void kill_part(int i)
{
	int x, y;

	if (parts[i].type != PT_PHOT) {
		x = (int)(parts[i].x+0.5f);
		y = (int)(parts[i].y+0.5f);
		if (parts[i].type == PT_STKM)
		{
			death = 1;
			isplayer = 0;
		}
		if (parts[i].type == PT_STKM2)
		{
			death2 = 1;
			isplayer2 = 0;
		}
		if (parts[i].type == PT_SPAWN)
		{
			ISSPAWN1 = 0;
		}
		if (parts[i].type == PT_SPAWN2)
		{
			ISSPAWN2 = 0;
		}
		if (x>=0 && y>=0 && x<XRES && y<YRES) {
			if ((pmap[y][x]>>8)==i)
				pmap[y][x] = 0;
			else if ((photons[y][x]>>8)==i)
				photons[y][x] = 0;
		}
	}

	parts[i].type = PT_NONE;
	parts[i].life = pfree;
	pfree = i;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void part_change_type(int n, int x, int y, int t)
#else
inline void part_change_type(int i, int x, int y, int t)
#endif
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || i>=NPART)
		return -1;
	parts[i].type = t;
	if (t!=PT_STKM&&t!=PT_STKM2 && t!=PT_PHOT)// && t!=PT_NEUT)
		pmap[y][x] = t|(i<<8);
	else if ((pmap[y][x]>>8)==i)
		pmap[y][x] = 0;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int create_n_parts(int n, int x, int y, int t)
#else
inline int create_n_parts(int n, int x, int y, float vx, float vy, int t)
#endif
{
	int i, c;
	n = (n/10);
	if (n<1) {
		n = 1;
	}
	if (n>680) {
		n = 680;
	}
	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return -1;

	for (c; c<n; c++) {
		float r = (rand()%128+128)/127.0f;
		float a = (rand()%360)*3.14159f/180.0f;
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;

		parts[i].x = (float)x;
		parts[i].y = (float)y;
		parts[i].type = t;
		parts[i].life = rand()%480+480;
		parts[i].vx = r*cosf(a);
		parts[i].vy = r*sinf(a);
		parts[i].ctype = 0;
		parts[i].temp += (n*17);
		parts[i].tmp = 0;
		if (t!=PT_STKM&&t!=PT_STKM2 && t!=PT_PHOT && !pmap[y][x])// && t!=PT_NEUT)
			pmap[y][x] = t|(i<<8);

		pv[y/CELL][x/CELL] += 6.0f * CFDS;
	}
	return 0;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int create_part(int p, int x, int y, int t)
#else
inline int create_part(int p, int x, int y, int t)
#endif
{
	int i;

	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return -1;

	if (t==SPC_HEAT||t==SPC_COOL)
	{
		if ((pmap[y][x]&0xFF)!=PT_NONE&&(pmap[y][x]&0xFF)<PT_NUM)
		{
			if (t==SPC_HEAT&&parts[pmap[y][x]>>8].temp<MAX_TEMP)
			{
				if ((pmap[y][x]&0xFF)==PT_PUMP) {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 0.1f, MIN_TEMP, MAX_TEMP);
				} else if ((sdl_mod & (KMOD_SHIFT)) && (sdl_mod & (KMOD_CTRL))) {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 50.0f, MIN_TEMP, MAX_TEMP);
				} else {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 4.0f, MIN_TEMP, MAX_TEMP);
				}
			}
			if (t==SPC_COOL&&parts[pmap[y][x]>>8].temp>MIN_TEMP)
			{
				if ((pmap[y][x]&0xFF)==PT_PUMP) {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp - 0.1f, MIN_TEMP, MAX_TEMP);
				} else if ((sdl_mod & (KMOD_SHIFT)) && (sdl_mod & (KMOD_CTRL))) {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp - 50.0f, MIN_TEMP, MAX_TEMP);
				} else {
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp - 4.0f, MIN_TEMP, MAX_TEMP);
				}
			}
			return pmap[y][x]>>8;
		}
		else
		{
			return -1;
		}
	}
	if (t==SPC_AIR)
	{
		pv[y/CELL][x/CELL] += 0.03f;
		if (y+CELL<YRES)
			pv[y/CELL+1][x/CELL] += 0.03f;
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] += 0.03f;
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] += 0.03f;
		}
		return -1;
	}
	if (t==SPC_VACUUM)
	{
		pv[y/CELL][x/CELL] -= 0.03f;
		if (y+CELL<YRES)
			pv[y/CELL+1][x/CELL] -= 0.03f;
		if (x+CELL<XRES)
		{
			pv[y/CELL][x/CELL+1] -= 0.03f;
			if (y+CELL<YRES)
				pv[y/CELL+1][x/CELL+1] -= 0.03f;
		}
		return -1;
	}

	if (t==PT_SPRK)
	{
		if (!((pmap[y][x]&0xFF)==PT_INST||(ptypes[pmap[y][x]&0xFF].properties&PROP_CONDUCTS))
			|| (pmap[y][x]&0xFF)==PT_QRTZ)
			return -1;
		if (parts[pmap[y][x]>>8].life!=0)
			return -1;
		parts[pmap[y][x]>>8].type = PT_SPRK;
		parts[pmap[y][x]>>8].life = 4;
		parts[pmap[y][x]>>8].ctype = pmap[y][x]&0xFF;
		pmap[y][x] = (pmap[y][x]&~0xFF) | PT_SPRK;
		return pmap[y][x]>>8;
	}
	if (t==PT_SPAWN&&ISSPAWN1)
		return -1;
	if (t==PT_SPAWN2&&ISSPAWN2)
		return -1;
	if (p==-1)//creating from anything but brush
	{
		if (pmap[y][x])
		{
			if ((pmap[y][x]&0xFF)!=PT_SPAWN&&(pmap[y][x]&0xFF)!=PT_SPAWN2)
			{
				if (t!=PT_STKM&&t!=PT_STKM2)
				{
					return -1;
				}
			}
		}
		if (photons[y][x] && t==PT_PHOT)
			return -1;
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else if (p==-2)//creating from brush
	{
		if (pmap[y][x])
		{
			if (((pmap[y][x]&0xFF)==PT_CLNE||(pmap[y][x]&0xFF)==PT_BCLN||((pmap[y][x]&0xFF)==PT_PCLN&&t!=PT_PSCN&&t!=PT_NSCN))&&(t!=PT_CLNE&&t!=PT_PCLN&&t!=PT_BCLN&&t!=PT_STKM&&t!=PT_STKM2))
			{
				parts[pmap[y][x]>>8].ctype = t;
			}
			return -1;
		}
		if (photons[y][x] && t==PT_PHOT)
			return -1;
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else
		i = p;

	if (t==PT_GLAS)
	{
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
	}
	if (t==PT_QRTZ)
	{
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
	}
	if (t!=PT_STKM&&t!=PT_STKM2)
	{
		parts[i].x = (float)x;
		parts[i].y = (float)y;
		parts[i].type = t;
		parts[i].vx = 0;
		parts[i].vy = 0;
		parts[i].life = 0;
		parts[i].ctype = 0;
		parts[i].temp = ptypes[t].heat;
		parts[i].tmp = 0;
	}
	if (t==PT_ACID)
	{
		parts[i].life = 75;
	}
	/*Testing
	if(t==PT_WOOD){
		parts[i].life = 150;
	}
	End Testing*/
	if (t==PT_WARP) {
		parts[i].life = rand()%95+70;
	}
	if (t==PT_FUSE) {
		parts[i].life = 50;
		parts[i].tmp = 50;
	}
	if (t==PT_DEUT)
		parts[i].life = 10;
	if (t==PT_BRAY)
		parts[i].life = 30;
	if (t==PT_PUMP)
		parts[i].life= 10;
	if (t==PT_SING)
		parts[i].life = rand()%50+60;
	if (t==PT_QRTZ)
		parts[i].tmp = (rand()%11);
	if (t==PT_PQRT)
		parts[i].tmp = (rand()%11);
	if (t==PT_FSEP)
		parts[i].life = 50;
	if (t==PT_COAL) {
		parts[i].life = 110;
		parts[i].tmp = 50;
	}
	if (t==PT_FRZW)
		parts[i].life = 100;
	if (t==PT_PIPE)
		parts[i].life = 60;
	if (t==PT_BCOL)
		parts[i].life = 110;
	if (t==PT_FIRE)
		parts[i].life = rand()%50+120;
	if (t==PT_PLSM)
		parts[i].life = rand()%150+50;
	if (t==PT_HFLM)
		parts[i].life = rand()%150+50;
	if (t==PT_LAVA)
		parts[i].life = rand()%120+240;
	if (t==PT_NBLE)
		parts[i].life = 0;
	if (t==PT_ICEI)
		parts[i].ctype = PT_WATR;
	if (t==PT_NEUT)
	{
		float r = (rand()%128+128)/127.0f;
		float a = (rand()%360)*3.14159f/180.0f;
		parts[i].life = rand()%480+480;
		parts[i].vx = r*cosf(a);
		parts[i].vy = r*sinf(a);
	}
	if (t==PT_MORT)
	{
		parts[i].vx = 2;
	}
	if (t==PT_PHOT)
	{
		float a = (rand()%8) * 0.78540f;
		parts[i].life = 680;
		parts[i].ctype = 0x3FFFFFFF;
		parts[i].vx = 3.0f*cosf(a);
		parts[i].vy = 3.0f*sinf(a);
	}
	if (t==PT_PHOT)
		photons[y][x] = t|(i<<8);
	if (t==PT_STKM)
	{
		if (isplayer==0)
		{
			if (pmap[y][x]&0xFF==PT_SPAWN)
			{
				parts[pmap[y][x]>>8].type = PT_STKM;
				parts[pmap[y][x]>>8].vx = 0;
				parts[pmap[y][x]>>8].vy = 0;
				parts[pmap[y][x]>>8].life = 100;
				parts[pmap[y][x]>>8].ctype = 0;
				parts[pmap[y][x]>>8].temp = ptypes[t].heat;

			}
			else
			{
				parts[i].x = (float)x;
				parts[i].y = (float)y;
				parts[i].type = PT_STKM;
				parts[i].vx = 0;
				parts[i].vy = 0;
				parts[i].life = 100;
				parts[i].ctype = 0;
				parts[i].temp = ptypes[t].heat;
			}



			player[3] = x-1;  //Setting legs positions
			player[4] = y+6;
			player[5] = x-1;
			player[6] = y+6;

			player[7] = x-3;
			player[8] = y+12;
			player[9] = x-3;
			player[10] = y+12;

			player[11] = x+1;
			player[12] = y+6;
			player[13] = x+1;
			player[14] = y+6;

			player[15] = x+3;
			player[16] = y+12;
			player[17] = x+3;
			player[18] = y+12;

			isplayer = 1;
		}
		else
		{
			return -1;
		}
		//kill_part(playerspawn);
		create_part(-1,x,y,PT_SPAWN);
		ISSPAWN1 = 1;
	}
	if (t==PT_STKM2)
	{
		if (isplayer2==0)
		{
			if (pmap[y][x]&0xFF==PT_SPAWN2)
			{
				parts[pmap[y][x]>>8].type = PT_STKM2;
				parts[pmap[y][x]>>8].vx = 0;
				parts[pmap[y][x]>>8].vy = 0;
				parts[pmap[y][x]>>8].life = 100;
				parts[pmap[y][x]>>8].ctype = 0;
				parts[pmap[y][x]>>8].temp = ptypes[t].heat;

			}
			else
			{
				parts[i].x = (float)x;
				parts[i].y = (float)y;
				parts[i].type = PT_STKM2;
				parts[i].vx = 0;
				parts[i].vy = 0;
				parts[i].life = 100;
				parts[i].ctype = 0;
				parts[i].temp = ptypes[t].heat;
			}



			player2[3] = x-1;  //Setting legs positions
			player2[4] = y+6;
			player2[5] = x-1;
			player2[6] = y+6;

			player2[7] = x-3;
			player2[8] = y+12;
			player2[9] = x-3;
			player2[10] = y+12;

			player2[11] = x+1;
			player2[12] = y+6;
			player2[13] = x+1;
			player2[14] = y+6;

			player2[15] = x+3;
			player2[16] = y+12;
			player2[17] = x+3;
			player2[18] = y+12;

			isplayer2 = 1;
		}
		else
		{
			return -1;
		}
		//kill_part(player2spawn);
		create_part(-1,x,y,PT_SPAWN2);
		ISSPAWN2 = 1;
	}
	if (t==PT_BIZR||t==PT_BIZRG)
		parts[i].ctype = 0x47FFFF;
	if (t!=PT_STKM&&t!=PT_STKM2 && t!=PT_PHOT)// && t!=PT_NEUT)  is this needed? it breaks floodfill, Yes photons should not be placed in the PMAP
		pmap[y][x] = t|(i<<8);

	return i;
}

static void create_gain_photon(int pp)
{
	float xx, yy;
	int i, lr, temp_bin, nx, ny;

	if (pfree == -1)
		return;
	i = pfree;

	lr = rand() % 2;

	if (lr) {
		xx = parts[pp].x - 0.3*parts[pp].vy;
		yy = parts[pp].y + 0.3*parts[pp].vx;
	} else {
		xx = parts[pp].x + 0.3*parts[pp].vy;
		yy = parts[pp].y - 0.3*parts[pp].vx;
	}

	nx = (int)(xx + 0.5f);
	ny = (int)(yy + 0.5f);

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return;

	if ((pmap[ny][nx] & 0xFF) != PT_GLOW)
		return;

	pfree = parts[i].life;

	parts[i].type = PT_PHOT;
	parts[i].life = 680;
	parts[i].x = xx;
	parts[i].y = yy;
	parts[i].vx = parts[pp].vx;
	parts[i].vy = parts[pp].vy;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].tmp = 0;
	photons[ny][nx] = PT_PHOT|(i<<8);

	temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
	if (temp_bin < 0) temp_bin = 0;
	if (temp_bin > 25) temp_bin = 25;
	parts[i].ctype = 0x1F << temp_bin;
}

static void create_cherenkov_photon(int pp)
{
	int i, lr, nx, ny;
	float r, eff_ior;

	if (pfree == -1)
		return;
	i = pfree;

	nx = (int)(parts[pp].x + 0.5f);
	ny = (int)(parts[pp].y + 0.5f);
	if ((pmap[ny][nx] & 0xFF) != PT_GLAS)
		return;

	if (hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
		return;

	pfree = parts[i].life;

	lr = rand() % 2;

	parts[i].type = PT_PHOT;
	parts[i].ctype = 0x00000F80;
	parts[i].life = 680;
	parts[i].x = parts[pp].x;
	parts[i].y = parts[pp].y;
	parts[i].temp = parts[pmap[ny][nx] >> 8].temp;
	parts[i].tmp = 0;
	photons[ny][nx] = PT_PHOT|(i<<8);

	if (lr) {
		parts[i].vx = parts[pp].vx - 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy + 2.5f*parts[pp].vx;
	} else {
		parts[i].vx = parts[pp].vx + 2.5f*parts[pp].vy;
		parts[i].vy = parts[pp].vy - 2.5f*parts[pp].vx;
	}

	/* photons have speed of light. no discussion. */
	r = 1.269 / hypotf(parts[i].vx, parts[i].vy);
	parts[i].vx *= r;
	parts[i].vy *= r;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void delete_part(int x, int y)
#else
inline void delete_part(int x, int y)
#endif
{
	unsigned i;

	if (x<0 || y<0 || x>=XRES || y>=YRES)
		return;
	if (photons[y][x]) {
		i = photons[y][x];
	} else {
		i = pmap[y][x];
	}

	if (!i || (i>>8)>=NPART)
		return;
	if ((parts[i>>8].type==SLALT)||SLALT==0)
	{
		kill_part(i>>8);
	}
	else if (ptypes[parts[i>>8].type].menusection==SEC)
	{
		kill_part(i>>8);
	}
	else
		return;
	
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int is_wire(int x, int y)
#else
inline int is_wire(int x, int y)
#endif
{
	return bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int is_wire_off(int x, int y)
#else
inline int is_wire_off(int x, int y)
#endif
{
	return (bmap[y][x]==WL_DETECT || bmap[y][x]==WL_EWALL || bmap[y][x]==WL_ALLOWLIQUID || bmap[y][x]==WL_WALLELEC || bmap[y][x]==WL_ALLOWALLELEC || bmap[y][x]==WL_EHOLE) && emap[y][x]<8;
}

int get_wavelength_bin(int *wm)
{
	int i, w0=30, wM=0;

	if (!*wm)
		return -1;

	for (i=0; i<30; i++)
		if (*wm & (1<<i)) {
			if (i < w0)
				w0 = i;
			if (i > wM)
				wM = i;
		}

	if (wM-w0 < 5)
		return (wM+w0)/2;

	i = rand() % (wM-w0-3);
	i += w0;

	*wm &= 0x1F << i;
	return i + 2;
}

void set_emap(int x, int y)
{
	int x1, x2;

	if (!is_wire_off(x, y))
		return;

	// go left as far as possible
	x1 = x2 = x;
	while (x1>0)
	{
		if (!is_wire_off(x1-1, y))
			break;
		x1--;
	}
	while (x2<XRES/CELL-1)
	{
		if (!is_wire_off(x2+1, y))
			break;
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
		emap[y][x] = 16;

	// fill children

	if (y>1 && x1==x2 &&
	        is_wire(x1-1, y-1) && is_wire(x1, y-1) && is_wire(x1+1, y-1) &&
	        !is_wire(x1-1, y-2) && is_wire(x1, y-2) && !is_wire(x1+1, y-2))
		set_emap(x1, y-2);
	else if (y>0)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y-1))
			{
				if (x==x1 || x==x2 || y>=YRES/CELL-1 ||
				        is_wire(x-1, y-1) || is_wire(x+1, y-1) ||
				        is_wire(x-1, y+1) || !is_wire(x, y+1) || is_wire(x+1, y+1))
					set_emap(x, y-1);
			}

	if (y<YRES/CELL-2 && x1==x2 &&
	        is_wire(x1-1, y+1) && is_wire(x1, y+1) && is_wire(x1+1, y+1) &&
	        !is_wire(x1-1, y+2) && is_wire(x1, y+2) && !is_wire(x1+1, y+2))
		set_emap(x1, y+2);
	else if (y<YRES/CELL-1)
		for (x=x1; x<=x2; x++)
			if (is_wire_off(x, y+1))
			{
				if (x==x1 || x==x2 || y<0 ||
				        is_wire(x-1, y+1) || is_wire(x+1, y+1) ||
				        is_wire(x-1, y-1) || !is_wire(x, y-1) || is_wire(x+1, y-1))
					set_emap(x, y+1);
			}
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int parts_avg(int ci, int ni,int t)//t is the particle you are looking for
#else
inline int parts_avg(int ci, int ni,int t)
#endif
{
	if (t==PT_INSL)//to keep electronics working
	{
		int pmr = pmap[(int)((parts[ci].y + parts[ni].y)/2)][(int)((parts[ci].x + parts[ni].x)/2)];
		if ((pmr>>8) < NPART && pmr)
			return parts[pmr>>8].type;
		else
			return PT_NONE;
	}
	else
	{
		int pmr2 = pmap[(int)((parts[ci].y + parts[ni].y)/2+0.5f)][(int)((parts[ci].x + parts[ni].x)/2+0.5f)];//seems to be more accurate.
		if ((pmr2>>8) < NPART && pmr2)
		{
			if (parts[pmr2>>8].type==t)
				return t;
		}
		else
			return PT_NONE;
	}
	return PT_NONE;
}


int nearest_part(int ci, int t)
{
	int distance = MAX_DISTANCE;
	int ndistance = 0;
	int id = -1;
	int i = 0;
	int cx = (int)parts[ci].x;
	int cy = (int)parts[ci].y;
	for (i=0; i<NPART; i++)
	{
		if (parts[i].type==t&&!parts[i].life&&i!=ci)
		{
			ndistance = abs((cx-parts[i].x)+(cy-parts[i].y));// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

void update_particles_i(pixel *vid, int start, int inc)
{
	int i, j, x, y, t, nx, ny, r, surround_space, s, lt, rt, nt, nnx, nny, q, golnum, goldelete, z;
	float mv, dx, dy, ix, iy, lx, ly, nrx, nry, dp;
	int fin_x, fin_y, clear_x, clear_y;
	float fin_xf, fin_yf, clear_xf, clear_yf;
	float nn, ct1, ct2;
	float pt = R_TEMP;
	float c_heat = 0.0f;
	int h_count = 0;
	int starti = (start*-1);
	int surround[8];
	int surround_hconduct[8];
	float pGravX, pGravY, pGravD;

	if (sys_pause&&!framerender)
		return;
	if (ISGRAV==1)
	{
		ISGRAV = 0;
		GRAV ++;
		GRAV_R = 60;
		GRAV_G = 0;
		GRAV_B = 0;
		GRAV_R2 = 30;
		GRAV_G2 = 30;
		GRAV_B2 = 0;
		for ( q = 0; q <= GRAV; q++)
		{
			if (GRAV_R >0 && GRAV_G==0)
			{
				GRAV_R--;
				GRAV_B++;
			}
			if (GRAV_B >0 && GRAV_R==0)
			{
				GRAV_B--;
				GRAV_G++;
			}
			if (GRAV_G >0 && GRAV_B==0)
			{
				GRAV_G--;
				GRAV_R++;
			}
			if (GRAV_R2 >0 && GRAV_G2==0)
			{
				GRAV_R2--;
				GRAV_B2++;
			}
			if (GRAV_B2 >0 && GRAV_R2==0)
			{
				GRAV_B2--;
				GRAV_G2++;
			}
			if (GRAV_G2 >0 && GRAV_B2==0)
			{
				GRAV_G2--;
				GRAV_R2++;
			}
		}
		if (GRAV>180) GRAV = 0;

	}
	if (ISLOVE==1)
	{
		ISLOVE = 0;
		for (ny=0; ny<YRES-4; ny++)
		{
			for (nx=0; nx<XRES-4; nx++)
			{
				r=pmap[ny][nx];
				if ((r>>8)>=NPART || !r)
				{
					continue;
				}
				else if ((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&parts[r>>8].type==PT_LOVE)
					kill_part(r>>8);
				else if (parts[r>>8].type==PT_LOVE)
				{
					love[nx/9][ny/9] = 1;
				}

			}
		}
		for (nx=9; nx<=XRES-18; nx++)
		{
			for (ny=9; ny<=YRES-7; ny++)
			{
				if (love[nx/9][ny/9]==1)
				{
					for ( nnx=0; nnx<9; nnx++)
						for ( nny=0; nny<9; nny++)
						{
							if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
							{
								rt=pmap[ny+nny][nx+nnx];
								if ((rt>>8)>=NPART)
								{
									continue;
								}
								if (!rt&&loverule[nnx][nny]==1)
									create_part(-1,nx+nnx,ny+nny,PT_LOVE);
								else if (!rt)
									continue;
								else if (parts[rt>>8].type==PT_LOVE&&loverule[nnx][nny]==0)
									kill_part(rt>>8);
							}
						}
				}
				love[nx/9][ny/9]=0;
			}
		}
	}
	if (ISLOLZ==1)
	{
		ISLOLZ = 0;
		for (ny=0; ny<YRES-4; ny++)
		{
			for (nx=0; nx<XRES-4; nx++)
			{
				r=pmap[ny][nx];
				if ((r>>8)>=NPART || !r)
				{
					continue;
				}
				else if ((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&parts[r>>8].type==PT_LOLZ)
					kill_part(r>>8);
				else if (parts[r>>8].type==PT_LOLZ)
				{
					lolz[nx/9][ny/9] = 1;
				}

			}
		}
		for (nx=9; nx<=XRES-18; nx++)
		{
			for (ny=9; ny<=YRES-7; ny++)
			{
				if (lolz[nx/9][ny/9]==1)
				{
					for ( nnx=0; nnx<9; nnx++)
						for ( nny=0; nny<9; nny++)
						{
							if (ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
							{
								rt=pmap[ny+nny][nx+nnx];
								if ((rt>>8)>=NPART)
								{
									continue;
								}
								if (!rt&&lolzrule[nny][nnx]==1)
									create_part(-1,nx+nnx,ny+nny,PT_LOLZ);
								else if (!rt)
									continue;
								else if (parts[rt>>8].type==PT_LOLZ&&lolzrule[nny][nnx]==0)
									kill_part(rt>>8);

							}
						}
				}
				lolz[nx/9][ny/9]=0;
			}
		}
	}
	if (ISGOL==1&&++CGOL>=GSPEED)//GSPEED is frames per generation
	{
		CGOL=0;
		ISGOL=0;
		int createdsomething = 0;
		for (nx=CELL; nx<XRES-CELL; nx++)
			for (ny=CELL; ny<YRES-CELL; ny++)
			{
				r = pmap[ny][nx];
				if ((r>>8)>=NPART || !r)
				{
					gol[nx][ny] = 0;
					continue;
				}
				else
					for ( golnum=1; golnum<NGOL; golnum++)
						if (parts[r>>8].type==goltype[golnum-1])
						{
							gol[nx][ny] = golnum;
							for ( nnx=-1; nnx<2; nnx++)
								for ( nny=-1; nny<2; nny++)//it will count itself as its own neighbor, which is needed, but will have 1 extra for delete check
								{
									gol2[((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL][((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL][golnum] ++;
									gol2[((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL][((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL][0] ++;
								}
						}
			}
		for (nx=CELL; nx<XRES-CELL; nx++)
			for (ny=CELL; ny<YRES-CELL; ny++)
			{
				int neighbors = gol2[nx][ny][0];
				if (neighbors==0)
					continue;
				for ( golnum = 1; golnum<NGOL; golnum++)
					for ( goldelete = 0; goldelete<9; goldelete++)
					{
						if (neighbors==goldelete&&gol[nx][ny]==0&&grule[golnum][goldelete]>=2&&gol2[nx][ny][golnum]>=(goldelete%2)+goldelete/2)
						{
							if (create_part(-1,nx,ny,goltype[golnum-1]))
								createdsomething = 1;
						}
						else if (neighbors-1==goldelete&&gol[nx][ny]==golnum&&(grule[golnum][goldelete]==0||grule[golnum][goldelete]==2))//subtract 1 because it counted itself
							kill_part(pmap[ny][nx]>>8);
					}
				gol2[nx][ny][0] = 0;
				for ( z = 1; z<NGOL; z++)
					gol2[nx][ny][z] = 0;
			}
		if (createdsomething)
			GENERATION ++;
	}
	if (ISWIRE==1)
	{
		for ( q = 0; q<(int)(MAX_TEMP-73.15f)/100+2; q++)
			if (!wireless[q][1])
			{
				wireless[q][0] = 0;
			}
			else
				wireless[q][1] = 0;
	}
	for (i=start; i<(NPART-starti); i+=inc)
		if (parts[i].type)
		{
			lx = parts[i].x;
			ly = parts[i].y;
			t = parts[i].type;
			//printf("parts[%d].type: %d\n", i, parts[i].type);


			if (parts[i].life && t!=PT_ACID  && t!=PT_COAL && t!=PT_WOOD && t!=PT_STKM && t!=PT_STKM2 && t!=PT_FUSE && t!=PT_FSEP && t!=PT_BCOL && t!=PT_GOL && t!=PT_SPNG && t!=PT_DEUT)
			{
				if (!(parts[i].life==10&&(t==PT_SWCH||t==PT_LCRY||t==PT_PCLN||t==PT_HSWC||t==PT_PUMP)))
					parts[i].life--;
				if (parts[i].life<=0 && !(ptypes[t].properties&PROP_CONDUCTS) && t!=PT_ARAY && t!=PT_FIRW && t!=PT_SWCH && t!=PT_PCLN && t!=PT_HSWC && t!=PT_PUMP && t!=PT_SPRK && t!=PT_LAVA && t!=PT_LCRY && t!=PT_QRTZ && t!=PT_GLOW && t!= PT_FOG && t!=PT_PIPE && t!=PT_FRZW &&(t!=PT_ICEI&&parts[i].ctype!=PT_FRZW)&&t!=PT_INST && t!=PT_SHLD1&& t!=PT_SHLD2&& t!=PT_SHLD3&& t!=PT_SHLD4)
				{
					kill_part(i);
					continue;
				}
			}

			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);


			if (x<CELL || y<CELL || x>=XRES-CELL || y>=YRES-CELL ||
			        (bmap[y/CELL][x/CELL] &&
			         (bmap[y/CELL][x/CELL]==WL_WALL ||
			          bmap[y/CELL][x/CELL]==WL_WALLELEC ||
			          bmap[y/CELL][x/CELL]==WL_ALLOWAIR ||
			          (bmap[y/CELL][x/CELL]==WL_DESTROYALL) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID && ptypes[t].falldown!=2) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWSOLID && ptypes[t].falldown!=1) ||
			          (bmap[y/CELL][x/CELL]==WL_ALLOWGAS && ptypes[t].falldown!=0 && parts[i].type!=PT_FIRE && parts[i].type!=PT_SMKE && parts[i].type!=PT_HFLM) ||
			          (bmap[y/CELL][x/CELL]==WL_DETECT && (t==PT_METL || t==PT_SPRK)) ||
			          (bmap[y/CELL][x/CELL]==WL_EWALL && !emap[y/CELL][x/CELL])) && (t!=PT_STKM) && (t!=PT_STKM2)))
			{
				kill_part(i);
				continue;
			}
			if (bmap[y/CELL][x/CELL]==WL_DETECT && emap[y/CELL][x/CELL]<8)
				set_emap(x/CELL, y/CELL);


			vx[y/CELL][x/CELL] = vx[y/CELL][x/CELL]*ptypes[t].airloss + ptypes[t].airdrag*parts[i].vx;
			vy[y/CELL][x/CELL] = vy[y/CELL][x/CELL]*ptypes[t].airloss + ptypes[t].airdrag*parts[i].vy;

			if (t==PT_GAS||t==PT_NBLE)
			{
				if (pv[y/CELL][x/CELL]<3.5f)
					pv[y/CELL][x/CELL] += ptypes[t].hotair*(3.5f-pv[y/CELL][x/CELL]);
				if (y+CELL<YRES && pv[y/CELL+1][x/CELL]<3.5f)
					pv[y/CELL+1][x/CELL] += ptypes[t].hotair*(3.5f-pv[y/CELL+1][x/CELL]);
				if (x+CELL<XRES)
				{
					pv[y/CELL][x/CELL+1] += ptypes[t].hotair*(3.5f-pv[y/CELL][x/CELL+1]);
					if (y+CELL<YRES)
						pv[y/CELL+1][x/CELL+1] += ptypes[t].hotair*(3.5f-pv[y/CELL+1][x/CELL+1]);
				}
			}
			else
			{
				pv[y/CELL][x/CELL] += ptypes[t].hotair;
				if (y+CELL<YRES)
					pv[y/CELL+1][x/CELL] += ptypes[t].hotair;
				if (x+CELL<XRES)
				{
					pv[y/CELL][x/CELL+1] += ptypes[t].hotair;
					if (y+CELL<YRES)
						pv[y/CELL+1][x/CELL+1] += ptypes[t].hotair;
				}
			}

			//Gravity mode by Moach
			switch (gravityMode)
			{
			default:
			case 0:
				pGravX = pGravY = 0.0f;
				break;
			case 1:
				pGravX = 0.0f;
				pGravY = ptypes[t].gravity;
				break;
			case 2:

				pGravD = 0.01f - hypotf((x - XCNTR), (y - YCNTR));

				pGravX = ptypes[t].gravity * ((float)(x - XCNTR) / pGravD);
				pGravY = ptypes[t].gravity * ((float)(y - YCNTR) / pGravD);

			}

			parts[i].vx *= ptypes[t].loss;
			parts[i].vy *= ptypes[t].loss;

			parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL] + pGravX;
			parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL] + pGravY;


			if (ptypes[t].diffusion)
			{
				parts[i].vx += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
				parts[i].vy += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
			}

			j = surround_space = nt = 0;
			for (nx=-1; nx<2; nx++)
				for (ny=-1; ny<2; ny++) {
					if (nx||ny) {
						surround[j] = r = pmap[y+ny][x+nx];
						j++;
						if (!bmap[(y+ny)/CELL][(x+nx)/CELL] || bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_STREAM)
						{
							if (!(r&0xFF))
								surround_space = 1;
							if ((r&0xFF)!=t)
								nt = 1;
						}
					}
				}

			if (!legacy_enable)
			{
				if (y-2 >= 0 && y-2 < YRES && (ptypes[t].properties&TYPE_LIQUID)) {
					float swappage;
					r = pmap[y-2][x];
					if (!((r>>8)>=NPART || !r || parts[i].type != (r&0xFF))) {
						if (parts[i].temp>parts[r>>8].temp) {
							swappage = parts[i].temp;
							parts[i].temp = parts[r>>8].temp;
							parts[r>>8].temp = swappage;
						}
					}
				}

				c_heat = 0.0f;
				h_count = 0;
				if (t&&(t!=PT_HSWC||parts[i].life==10)&&ptypes[t].hconduct>(rand()%250))
				{
					for (j=0; j<8; j++)
					{
						surround_hconduct[j] = i;
						r = surround[j];
						if ((r>>8)>=NPART || !r)
							continue;
						rt = r&0xFF;
						if (rt&&ptypes[rt].hconduct&&(rt!=PT_HSWC||parts[r>>8].life==10)
						        &&(t!=PT_FILT||(rt!=PT_BRAY&&rt!=PT_BIZR&&rt!=PT_BIZRG))
						        &&(rt!=PT_FILT||(t!=PT_BRAY&&t!=PT_PHOT&&t!=PT_BIZR&&t!=PT_BIZRG)))
						{
							surround_hconduct[j] = r>>8;
							c_heat += parts[r>>8].temp;
							h_count++;
						}
					}

					pt = parts[i].temp = (c_heat+parts[i].temp)/(h_count+1);
					for (j=0; j<8; j++)
					{
						parts[surround_hconduct[j]].temp = pt;
					}

					s = 1;
					if (pt>ptransitions[t].thv&&ptransitions[t].tht>-1) {
						// particle type change due to high temperature
						if (ptransitions[t].tht!=PT_NUM)
							t = ptransitions[t].tht;
						else if (t==PT_ICEI) {
							if (parts[i].ctype&&parts[i].ctype!=PT_ICEI) {
								if (ptransitions[parts[i].ctype].tlt==PT_ICEI&&pt<=ptransitions[parts[i].ctype].tlv) s = 0;
								else {
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									parts[i].life = 0;
								}
							}
							else if (pt>274.0f) t = PT_WATR;
							else s = 0;
						}
						else if (t==PT_SLTW) {
							if (1>rand()%6) t = PT_SALT;
							else t = PT_WTRV;
						}
						else s = 0;
					} else if (pt<ptransitions[t].tlv&&ptransitions[t].tlt>-1) {
						// particle type change due to low temperature
						if (ptransitions[t].tlt!=PT_NUM)
							t = ptransitions[t].tlt;
						else if (t==PT_WTRV) {
							if (pt<273.0f) t = PT_RIME;
							else t = PT_DSTW;
						}
						else if (t==PT_LAVA) {
							if (parts[i].ctype&&parts[i].ctype!=PT_LAVA) {
								if (ptransitions[parts[i].ctype].tht==PT_LAVA&&pt>=ptransitions[parts[i].ctype].thv) s = 0;
								else if (parts[i].ctype==PT_THRM&&pt>=ptransitions[PT_BMTL].thv) s = 0;
								else {
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									if (t==PT_THRM) {
										parts[i].tmp = 0;
										t = PT_BMTL;
									}
									if (parts[i].ctype==PT_PLUT)
									{
										parts[i].tmp = 0;
										t = parts[i].ctype = PT_LAVA;
									}
								}
							}
							else if (pt<973.0f) t = PT_STNE;
							else s = 0;
						}
						else s = 0;
						if (s) parts[i].life = 0;
					}
					else s = 0;
					if (s) { // particle type change occurred
						parts[i].life = 0;
						if (t==PT_ICEI||t==PT_LAVA)
							parts[i].ctype = parts[i].type;
						if (ptypes[t].state==ST_GAS&&ptypes[parts[i].type].state!=ST_GAS)
							pv[y/CELL][x/CELL] += 0.50f;
						part_change_type(i,x,y,t);
						if (t==PT_FIRE||t==PT_PLSM||t==PT_HFLM)
							parts[i].life = rand()%50+120;
						if (t==PT_LAVA) {
							if (parts[i].ctype==PT_BRMT) parts[i].ctype = PT_BMTL;
							else if (parts[i].ctype==PT_SAND) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype==PT_BGLA) parts[i].ctype = PT_GLAS;
							else if (parts[i].ctype==PT_PQRT) parts[i].ctype = PT_QRTZ;
							parts[i].life = rand()%120+240;
						}
						if (t==PT_NONE) {
							kill_part(i);
							goto killed;
						}
					}

					pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
					if (t==PT_LAVA) {
						parts[i].life = restrict_flt((parts[i].temp-700)/7, 0.0f, 400.0f);
						if (parts[i].ctype==PT_THRM&&parts[i].tmp>0)
						{
							parts[i].tmp--;
							parts[i].temp = 3500;
						}
						if (parts[i].ctype==PT_PLUT&&parts[i].tmp>0)
						{
							parts[i].tmp--;
							parts[i].temp = MAX_TEMP;
						}
					}
				}
			}

			if (ptypes[t].properties&PROP_LIFE)
			{
				parts[i].temp = restrict_flt(parts[i].temp-50.0f, MIN_TEMP, MAX_TEMP);
				ISGOL=1;
			}
			if ((ptypes[t].properties&PROP_CONDUCTS) || t==PT_SPRK)
			{
				nx = x % CELL;
				if (nx == 0)
					nx = x/CELL - 1;
				else if (nx == CELL-1)
					nx = x/CELL + 1;
				else
					nx = x/CELL;
				ny = y % CELL;
				if (ny == 0)
					ny = y/CELL - 1;
				else if (ny == CELL-1)
					ny = y/CELL + 1;
				else
					ny = y/CELL;
				if (nx>=0 && ny>=0 && nx<XRES/CELL && ny<YRES/CELL)
				{
					if (t!=PT_SPRK)
					{
						if (emap[ny][nx]==12 && !parts[i].life)
						{
							part_change_type(i,x,y,PT_SPRK);
							parts[i].life = 4;
							parts[i].ctype = t;
							t = PT_SPRK;
						}
					}
					else if (bmap[ny][nx]==WL_DETECT || bmap[ny][nx]==WL_EWALL || bmap[ny][nx]==WL_ALLOWLIQUID || bmap[ny][nx]==WL_WALLELEC || bmap[ny][nx]==WL_ALLOWALLELEC || bmap[ny][nx]==WL_EHOLE)
						set_emap(nx, ny);
				}
			}


			if ((ptypes[t].explosive&2) && pv[y/CELL][x/CELL]>2.5f)
			{
				parts[i].life = rand()%80+180;
				parts[i].temp = restrict_flt(ptypes[PT_FIRE].heat + (ptypes[t].flammable/2), MIN_TEMP, MAX_TEMP);
				t = PT_FIRE;
				part_change_type(i,x,y,t);
				pv[y/CELL][x/CELL] += 0.25f * CFDS;
			}


			s = 1;
			if (pv[y/CELL][x/CELL]>ptransitions[t].phv&&ptransitions[t].pht>-1) {
				// particle type change due to high pressure
				if (ptransitions[t].pht!=PT_NUM)
					t = ptransitions[t].pht;
				else if (t==PT_BMTL) {
					if (pv[y/CELL][x/CELL]>2.5f)
						t = PT_BRMT;
					else if (pv[y/CELL][x/CELL]>1.0f && parts[i].tmp==1)
						t = PT_BRMT;
					else s = 0;
				}
				else s = 0;
			} else if (pv[y/CELL][x/CELL]<ptransitions[t].plv&&ptransitions[t].plt>-1) {
				// particle type change due to low pressure
				if (ptransitions[t].plt!=PT_NUM)
					t = ptransitions[t].plt;
				else s = 0;
			}
			else s = 0;
			if (s) { // particle type change occurred
				parts[i].life = 0;
				part_change_type(i,x,y,t);
				if (t==PT_FIRE)
					parts[i].life = rand()%50+120;
				if (t==PT_NONE) {
					kill_part(i);
					goto killed;
				}
			}

			if (ptypes[t].update_func)
			{
				if ((*(ptypes[t].update_func))(i,x,y,surround_space))
					continue;
			}
			if (legacy_enable)
				update_legacy_all(i,x,y,surround_space);

killed:
			if (parts[i].type == PT_NONE)
				continue;

			if (!parts[i].vx&&!parts[i].vy)
				continue;

#if defined(WIN32) && !defined(__GNUC__)
			mv = max(fabsf(parts[i].vx), fabsf(parts[i].vy));
#else
			mv = fmaxf(fabsf(parts[i].vx), fabsf(parts[i].vy));
#endif
			if (mv < ISTP)
			{
				clear_x = x;
				clear_y = y;
				clear_xf = parts[i].x;
				clear_yf = parts[i].y;
				fin_xf = clear_xf + parts[i].vx;
				fin_yf = clear_yf + parts[i].vy;
				fin_x = (int)(fin_xf+0.5f);
				fin_y = (int)(fin_yf+0.5f);
			}
			else
			{
				// interpolate to see if there is anything in the way
				dx = parts[i].vx*ISTP/mv;
				dy = parts[i].vy*ISTP/mv;
				fin_xf = parts[i].x;
				fin_yf = parts[i].y;
				while (1)
				{
					mv -= ISTP;
					fin_xf += dx;
					fin_yf += dy;
					fin_x = (int)(fin_xf+0.5f);
					fin_y = (int)(fin_yf+0.5f);
					if (mv <= 0.0f)
					{
						// nothing found
						fin_xf = parts[i].x + parts[i].vx;
						fin_yf = parts[i].y + parts[i].vy;
						fin_x = (int)(fin_xf+0.5f);
						fin_y = (int)(fin_yf+0.5f);
						clear_xf = fin_xf-dx;
						clear_yf = fin_yf-dy;
						clear_x = (int)(clear_xf+0.5f);
						clear_y = (int)(clear_yf+0.5f);
						break;
					}
					if (fin_x<CELL || fin_y<CELL || fin_x>=XRES-CELL || fin_y>=YRES-CELL || pmap[fin_y][fin_x] || (bmap[fin_y/CELL][fin_x/CELL] && bmap[fin_y/CELL][fin_x/CELL]!=WL_STREAM))
					{
						// found an obstacle
						clear_xf = fin_xf-dx;
						clear_yf = fin_yf-dy;
						clear_x = (int)(clear_xf+0.5f);
						clear_y = (int)(clear_yf+0.5f);
						break;
					}

				}
			}

			if ((t==PT_PHOT||t==PT_NEUT)) {
				if (t == PT_PHOT) {
					rt = pmap[fin_y][fin_x] & 0xFF;
					lt = pmap[y][x] & 0xFF;

					r = eval_move(PT_PHOT, fin_x, fin_y, NULL);
					if (((rt==PT_GLAS && lt!=PT_GLAS) || (rt!=PT_GLAS && lt==PT_GLAS)) && r) {
						if (!get_normal_interp(REFRACT|t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy, &nrx, &nry)) {
							kill_part(i);
							continue;
						}

						r = get_wavelength_bin(&parts[i].ctype);
						if (r == -1) {
							kill_part(i);
							continue;
						}
						nn = GLASS_IOR - GLASS_DISP*(r-15)/15.0f;
						nn *= nn;
						nrx = -nrx;
						nry = -nry;
						if (rt==PT_GLAS && lt!=PT_GLAS)
							nn = 1.0f/nn;
						ct1 = parts[i].vx*nrx + parts[i].vy*nry;
						ct2 = 1.0f - (nn*nn)*(1.0f-(ct1*ct1));
						if (ct2 < 0.0f) {
							// total internal reflection
							parts[i].vx -= 2.0f*ct1*nrx;
							parts[i].vy -= 2.0f*ct1*nry;
							fin_xf = parts[i].x;
							fin_yf = parts[i].y;
							fin_x = x;
							fin_y = y;
						} else {
							// refraction
							ct2 = sqrtf(ct2);
							ct2 = ct2 - nn*ct1;
							parts[i].vx = nn*parts[i].vx + ct2*nrx;
							parts[i].vy = nn*parts[i].vy + ct2*nry;
						}
					}
				}
				if (try_move(i, x, y, fin_x, fin_y)) {
					parts[i].x = fin_xf;
					parts[i].y = fin_yf;
				} else {
					// reflection
					parts[i].flags |= FLAG_STAGNANT;
					if (t==PT_NEUT && 100>(rand()%1000))
					{
						kill_part(i);
						continue;
					}
					r = pmap[fin_y][fin_x];

					// this should be replaced with a particle type attribute ("photwl" or something)
					if ((r & 0xFF) == PT_PSCN) parts[i].ctype  = 0x00000000;
					if ((r & 0xFF) == PT_NSCN) parts[i].ctype  = 0x00000000;
					if ((r & 0xFF) == PT_SPRK) parts[i].ctype  = 0x00000000;
					if ((r & 0xFF) == PT_COAL) parts[i].ctype  = 0x00000000;
					if ((r & 0xFF) == PT_BCOL) parts[i].ctype  = 0x00000000;
					if ((r & 0xFF) == PT_PLEX) parts[i].ctype &= 0x1F00003E;
					if ((r & 0xFF) == PT_NITR) parts[i].ctype &= 0x0007C000;
					if ((r & 0xFF) == PT_NBLE) parts[i].ctype &= 0x3FFF8000;
					if ((r & 0xFF) == PT_LAVA) parts[i].ctype &= 0x3FF00000;
					if ((r & 0xFF) == PT_ACID) parts[i].ctype &= 0x1FE001FE;
					if ((r & 0xFF) == PT_DUST) parts[i].ctype &= 0x3FFFFFC0;
					if ((r & 0xFF) == PT_SNOW) parts[i].ctype &= 0x03FFFFFF;
					if ((r & 0xFF) == PT_GOO)  parts[i].ctype &= 0x3FFAAA00;
					if ((r & 0xFF) == PT_PLNT) parts[i].ctype &= 0x0007C000;
					if ((r & 0xFF) == PT_PLUT) parts[i].ctype &= 0x001FCE00;
					if ((r & 0xFF) == PT_URAN) parts[i].ctype &= 0x003FC000;

					if (get_normal_interp(t, parts[i].x, parts[i].y, parts[i].vx, parts[i].vy, &nrx, &nry)) {
						dp = nrx*parts[i].vx + nry*parts[i].vy;
						parts[i].vx -= 2.0f*dp*nrx;
						parts[i].vy -= 2.0f*dp*nry;
						fin_x = (int)(parts[i].x + parts[i].vx + 0.5f);
						fin_y = (int)(parts[i].y + parts[i].vy + 0.5f);
						// cast as int then back to float for compatibility with existing saves
						if (try_move(i, x, y, fin_x, fin_y)) {
							parts[i].x = (float)fin_x;
							parts[i].y = (float)fin_y;
						} else {
							kill_part(i);
							continue;
						}
					} else {
						if (t!=PT_NEUT)
							kill_part(i);
						continue;
					}
					if (!parts[i].ctype&&t!=PT_NEUT) {
						kill_part(i);
						continue;
					}
				}
			}
			else if (ptypes[t].falldown==0)
			{
				// gasses and solids (but not powders)
				if (try_move(i, x, y, fin_x, fin_y)) {
					parts[i].x = fin_xf;
					parts[i].y = fin_yf;
				} else {
					// TODO
					if (fin_x>x+ISTP) fin_x=x+ISTP;
					if (fin_x<x-ISTP) fin_x=x-ISTP;
					if (fin_y>y+ISTP) fin_y=y+ISTP;
					if (fin_y<y-ISTP) fin_y=y-ISTP;
					if (try_move(i, x, y, 2*x-fin_x, fin_y))
					{
						parts[i].x = 0.25f+(float)(2*x-fin_x);
						parts[i].y = 0.25f+fin_y;
						parts[i].vx *= ptypes[t].collision;
					}
					else if (try_move(i, x, y, fin_x, 2*y-fin_y))
					{
						parts[i].x = 0.25f+fin_x;
						parts[i].y = 0.25f+(float)(2*y-fin_y);
						parts[i].vy *= ptypes[t].collision;
					}
					else
					{
						parts[i].vx *= ptypes[t].collision;
						parts[i].vy *= ptypes[t].collision;
					}
				}
			}
			else
			{
				// liquids and powders
				// TODO: rewrite to operate better with radial gravity
				if (try_move(i, x, y, fin_x, fin_y)) {
					parts[i].x = fin_xf;
					parts[i].y = fin_yf;
				} else {
					if (fin_x!=x && try_move(i, x, y, fin_x, clear_y))
					{
						parts[i].x = fin_xf;
						parts[i].y = clear_yf;
						parts[i].vx *= ptypes[t].collision;
						parts[i].vy *= ptypes[t].collision;
					}
					else if (fin_y!=y && try_move(i, x, y, clear_x, fin_y))
					{
						parts[i].x = clear_xf;
						parts[i].y = fin_yf;
						parts[i].vx *= ptypes[t].collision;
						parts[i].vy *= ptypes[t].collision;
					}
					else
					{
						r = (rand()%2)*2-1;
						if (fin_y!=clear_y && try_move(i, x, y, clear_x+r, fin_y))
						{
							parts[i].x = clear_xf+r;
							parts[i].y = fin_yf;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
						else if (fin_y!=clear_y && try_move(i, x, y, clear_x-r, fin_y))
						{
							parts[i].x = clear_xf-r;
							parts[i].y = fin_yf;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
						else if (fin_x!=clear_x && try_move(i, x, y, fin_x, clear_y+r))
						{
							parts[i].x = fin_xf;
							parts[i].y = clear_yf+r;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
						else if (fin_x!=clear_x && try_move(i, x, y, fin_x, clear_y-r))
						{
							parts[i].x = fin_xf;
							parts[i].y = clear_yf-r;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
						else if (ptypes[t].falldown>1 && (parts[i].vy>fabs(parts[i].vx) || gravityMode==2))
						{
							s = 0;
							if (!rt || nt) //nt is if there is an something else besides the current particle type, around the particle
								rt = 30;//slight less water lag, although it changes how it moves a lot
							else
								rt = 10;
							for (j=clear_x+r; j>=0 && j>=clear_x-rt && j<clear_x+rt && j<XRES; j+=r)
							{
								if (try_move(i, x, y, j, fin_y))
								{
									parts[i].x = clear_xf+(j-clear_x);
									parts[i].y = fin_yf;
									x = j;
									y = fin_y;
									s = 1;
									break;
								}
								if (try_move(i, x, y, j, clear_y))
								{
									parts[i].x = clear_xf+(j-clear_x);
									x = j;
									s = 1;
									break;
								}
								if ((pmap[y][j]&255)!=t || (bmap[y/CELL][j/CELL] && bmap[y/CELL][j/CELL]!=WL_STREAM))
									break;
							}
							if (parts[i].vy>0)
								r = 1;
							else
								r = -1;
							if (s)
								for (j=clear_y+r; j>=0 && j<YRES && j>=clear_y-rt && j<clear_y+rt; j+=r)
								{
									if (try_move(i, x, y, clear_x, j))
									{
										parts[i].y = clear_yf+(j-clear_y);
										break;
									}
									if ((pmap[j][x]&255)!=t || (bmap[j/CELL][x/CELL] && bmap[j/CELL][x/CELL]!=WL_STREAM))
									{
										s = 0;
										break;
									}
								}
							else if (clear_x!=x&&clear_y!=y && try_move(i, x, y, clear_x, clear_y)) {
								// if interpolation was done and haven't yet moved, try moving to last clear position
								parts[i].x = clear_xf;
								parts[i].y = clear_yf;
							}
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
							if (!s)
								parts[i].flags |= FLAG_STAGNANT;
						}
						else
						{
							if (clear_x!=x&&clear_y!=y && try_move(i, x, y, clear_x, clear_y)) {
								// if interpolation was done, try moving to last clear position
								parts[i].x = clear_xf;
								parts[i].y = clear_yf;
							}
							parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
					}
				}
			}
			nx = (int)(parts[i].x+0.5f);
			ny = (int)(parts[i].y+0.5f);
			if (nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)
			{
				kill_part(i);
				continue;
			}
		}
	if (framerender) {
		framerender = 0;
		sys_pause = 1;
	}
}

void update_particles(pixel *vid)
{
	int i, j, x, y, t, nx, ny, r, cr,cg,cb, l = -1;
	float lx, ly;
#ifdef MT
	int pt = 0, pc = 0;
	pthread_t *InterThreads;
#endif

	isplayer = 0;  //Needed for player spawning
	isplayer2 = 0;
	memset(pmap, 0, sizeof(pmap));
	memset(photons, 0, sizeof(photons));
	r = rand()%2;
	NUM_PARTS = 0;
	for (j=0; j<NPART; j++)
	{
		i = r ? (NPART-1-j) : j;
		if (parts[i].type)
		{
			t = parts[i].type;
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=0 && y>=0 && x<XRES && y<YRES && t!=PT_PHOT) {
				if (t!=PT_NEUT || (pmap[y][x]&0xFF)!=PT_GLAS)
					pmap[y][x] = t|(i<<8);
			}
			if (t==PT_PHOT)
				photons[y][x] = t|(i<<8);
			NUM_PARTS ++;
		}
		else
		{
			parts[i].life = l;
			l = i;
		}
	}
	pfree=l;
	if (cmode==CM_BLOB)
	{
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
			{
				if (bmap[y][x]==WL_WALL)
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);

						}
				if (bmap[y][x]==WL_DESTROYALL)
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
						}
				if (bmap[y][x]==WL_ALLOWLIQUID)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0xC0, 0xC0, 0xC0);
							}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_FAN)
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0xFF);
						}
				if (bmap[y][x]==WL_DETECT)
				{
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFF8080);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0xFF, 0x80, 0x80);
						}
					if (emap[y][x])
					{
						cr = 255;
						cg = 32;
						cb = 8;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_EWALL)
				{
					if (emap[y][x])
					{
						cr = cg = cb = 128;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (i&j&1)
								{
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
								}
					}
					else
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (!(i&j&1))
								{
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
									drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
								}
					}
				}
				if (bmap[y][x]==WL_WALLELEC)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0xC0, 0xC0, 0xC0);
							}
							else
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
							}
						}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_ALLOWALLELEC)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							//pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFFFF22);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0xFF, 0xFF, 0x22);
							}

						}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_ALLOWGAS)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x579777);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x57, 0x97, 0x77);
						}
					}
				}
				if (bmap[y][x]==WL_ALLOWAIR)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x3C3C3C);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x3C, 0x3C, 0x3C);
						}
					}
				}
				if (bmap[y][x]==WL_ALLOWSOLID)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x575757);
							drawblob(vid, (x*CELL+i), (y*CELL+j), 0x57, 0x57, 0x57);
						}
					}
				}
				if (bmap[y][x]==WL_EHOLE)
				{
					if (emap[y][x])
					{
						for (j=0; j<CELL; j++)
						{
							for (i=(j)&1; i<CELL; i++)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
							}
						}
						for (j=0; j<CELL; j+=2)
						{
							for (i=(j)&1; i<CELL; i+=2)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
							}
						}
					}
					else
					{
						for (j=0; j<CELL; j+=2)
						{
							for (i=(j)&1; i<CELL; i+=2)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
								drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
							}
						}
					}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (emap[y][x] && (!sys_pause||framerender))
					emap[y][x] --;
			}
		}
	}
	else
	{
		for (y=0; y<YRES/CELL; y++)
		{
			for (x=0; x<XRES/CELL; x++)
			{
				if (bmap[y][x]==WL_WALL)
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
						}
				if (bmap[y][x]==WL_DESTROYALL)
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
				if (bmap[y][x]==WL_ALLOWLIQUID)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_FAN)
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
				if (bmap[y][x]==WL_DETECT)
				{
					for (j=0; j<CELL; j+=2)
						for (i=(j>>1)&1; i<CELL; i+=2)
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFF8080);
					if (emap[y][x])
					{
						cr = 255;
						cg = 32;
						cb = 8;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_EWALL)
				{
					if (emap[y][x])
					{
						cr = cg = cb = 128;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (i&j&1)
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
					}
					else
					{
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
						for (j=0; j<CELL; j++)
							for (i=0; i<CELL; i++)
								if (!(i&j&1))
									vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
					}
				}
				if (bmap[y][x]==WL_WALLELEC)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
							else
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
						}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_ALLOWALLELEC)
				{
					for (j=0; j<CELL; j++)
						for (i=0; i<CELL; i++)
						{
							//pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
							if (!((y*CELL+j)%2) && !((x*CELL+i)%2))
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFFFF22);

						}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (bmap[y][x]==WL_ALLOWAIR)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x3C3C3C);
						}
					}
				}
				if (bmap[y][x]==WL_ALLOWGAS)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x579777);
						}
					}
				}
				if (bmap[y][x]==WL_ALLOWSOLID)
				{
					for (j=0; j<CELL; j+=2)
					{
						for (i=(j>>1)&1; i<CELL; i+=2)
						{
							vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x575757);
						}
					}
				}
				if (bmap[y][x]==WL_EHOLE)
				{
					if (emap[y][x])
					{
						for (j=0; j<CELL; j++)
						{
							for (i=(j)&1; i<CELL; i++)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
							}
						}
						for (j=0; j<CELL; j+=2)
						{
							for (i=(j)&1; i<CELL; i+=2)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
							}
						}
					}
					else
					{
						for (j=0; j<CELL; j+=2)
						{
							for (i=(j)&1; i<CELL; i+=2)
							{
								vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
							}
						}
					}
					if (emap[y][x])
					{
						cr = cg = cb = 16;
						cr += fire_r[y][x];
						if (cr > 255) cr = 255;
						fire_r[y][x] = cr;
						cg += fire_g[y][x];
						if (cg > 255) cg = 255;
						fire_g[y][x] = cg;
						cb += fire_b[y][x];
						if (cb > 255) cb = 255;
						fire_b[y][x] = cb;
					}
				}
				if (emap[y][x] && (!sys_pause||framerender))
					emap[y][x] --;
			}
		}
	}

	update_particles_i(vid, 0, 1);

	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
			if (bmap[y][x]==WL_STREAM)
			{
				lx = x*CELL + CELL*0.5f;
				ly = y*CELL + CELL*0.5f;
				for (t=0; t<1024; t++)
				{
					nx = (int)(lx+0.5f);
					ny = (int)(ly+0.5f);
					if (nx<0 || nx>=XRES || ny<0 || ny>=YRES)
						break;
					addpixel(vid, nx, ny, 255, 255, 255, 64);
					i = nx/CELL;
					j = ny/CELL;
					lx += vx[j][i]*0.125f;
					ly += vy[j][i]*0.125f;
					if (bmap[j][i]==WL_STREAM && i!=x && j!=y)
						break;
				}
				drawtext(vid, x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
			}

}

void rotate_area(int area_x, int area_y, int area_w, int area_h, int invert)
{
	//TODO: MSCC doesn't like arrays who's size is determined at runtime.
#if !(defined(WIN32) && !defined(__GNUC__))
	int cx = 0;
	int cy = 0;
	unsigned tpmap[area_h][area_w];
	unsigned rtpmap[area_w][area_h];
	unsigned char tbmap[area_h/CELL][area_w/CELL];
	unsigned char rtbmap[area_w/CELL][area_h/CELL];
	float tfvy[area_h/CELL][area_w/CELL];
	float tfvx[area_h/CELL][area_w/CELL];
	for (cy=0; cy<area_h; cy++)
	{
		for (cx=0; cx<area_w; cx++)//save walls to temp
		{
			if (area_x + cx<XRES&&area_y + cy<YRES)
			{
				if (bmap[(cy+area_y)/CELL][(cx+area_x)/CELL]) {
					tbmap[cy/CELL][cx/CELL] = bmap[(cy+area_y)/CELL][(cx+area_x)/CELL];
					if (bmap[(cy+area_y)/CELL][(cx+area_x)/CELL]==WL_FAN) {
						tfvx[cy/CELL][cx/CELL] = fvx[(cy+area_y)/CELL][(cx+area_x)/CELL];
						tfvy[cy/CELL][cx/CELL] = fvy[(cy+area_y)/CELL][(cx+area_x)/CELL];
					}
				} else {
					tbmap[cy/CELL][cx/CELL] = 0;
					tfvx[cy/CELL][cx/CELL] = 0;
					tfvy[cy/CELL][cx/CELL] = 0;
				}
			}
		}
	}
	for (cy=0; cy<area_h; cy++)
	{
		for (cx=0; cx<area_w; cx++)//save particles to temp
		{
			if ((area_x + cx<XRES&&area_y + cy<YRES))
			{
				tpmap[cy][cx] = pmap[(int)(cy+area_y+0.5f)][(int)(cx+area_x+0.5f)];
			}
			else
				tpmap[(int)(cy+0.5f)][(int)(cx+0.5f)] = 0;
		}
	}
	for (cy=0; cy<area_w; cy++)
	{
		for (cx=0; cx<area_h; cx++)//rotate temp arrays
		{
			if (invert)
			{
				rtbmap[cy/CELL][((area_h-1)-cx)/CELL] = tbmap[cy/CELL][cx/CELL];
				rtpmap[cy][(area_h-1)-cx] = tpmap[(int)(cy+0.5f)][(int)(cx+0.5f)];
				tfvx[cy/CELL][((area_h-1)-cx)/CELL] = -tfvx[cy/CELL][cx/CELL];
				tfvy[cy/CELL][((area_h-1)-cx)/CELL] = tfvy[cy/CELL][cx/CELL];
			}
			else
			{
				rtbmap[((area_h-1)-cx)/CELL][cy/CELL] = tbmap[cy/CELL][cx/CELL];
				rtpmap[(area_h-1)-cx][cy] = tpmap[(int)(cy+0.5f)][(int)(cx+0.5f)];
				tfvy[((area_h-1)-cx)/CELL][cy/CELL] = -tfvx[cy/CELL][cx/CELL];
				tfvx[((area_h-1)-cx)/CELL][cy/CELL] = tfvy[cy/CELL][cx/CELL];
			}
		}
	}
	for (cy=0; cy<area_w; cy++)
	{
		for (cx=0; cx<area_h; cx++)//move particles and walls
		{
			if (area_x + cx<XRES&&area_y + cy<YRES)
			{
				if ((rtpmap[cy][cx]>>8)<=NPART&&rtpmap[cy][cx])
				{
					parts[rtpmap[(int)(cy+0.5f)][(int)(cx+0.5f)]>>8].x = area_x +cx;
					parts[rtpmap[(int)(cy+0.5f)][(int)(cx+0.5f)]>>8].y = area_y +cy;
				}
				bmap[(area_y+cy)/CELL][(area_x+cx)/CELL] = rtbmap[cy/CELL][cx/CELL];
				fvy[(area_y+cy)/CELL][(area_x+cx)/CELL] = tfvy[cy/CELL][cx/CELL];
				fvx[(area_y+cy)/CELL][(area_x+cx)/CELL] = tfvx[cy/CELL][cx/CELL];
			}
		}
	}
#endif
}

void clear_area(int area_x, int area_y, int area_w, int area_h)
{
	int cx = 0;
	int cy = 0;
	for (cy=0; cy<area_h; cy++)
	{
		for (cx=0; cx<area_w; cx++)
		{
			bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
			delete_part(cx+area_x, cy+area_y);
		}
	}
}

void create_box(int x1, int y1, int x2, int y2, int c)
{
	int i, j;
	if (x1>x2)
	{
		i = x2;
		x2 = x1;
		x1 = i;
	}
	if (y1>y2)
	{
		j = y2;
		y2 = y1;
		y1 = j;
	}
	for (j=y1; j<=y2; j++)
		for (i=x1; i<=x2; i++)
			create_parts(i, j, 1, 1, c);
}

int flood_parts(int x, int y, int c, int cm, int bm)
{
	int x1, x2, dy = (c<PT_NUM)?1:CELL;
	int co = c, wall;
	if (cm==PT_INST&&co==PT_SPRK)
		if ((pmap[y][x]&0xFF)==PT_SPRK)
			return 0;
	if (c>=UI_WALLSTART&&c<=UI_WALLSTART+UI_WALLCOUNT)
	{
		wall = c-100;
	}
	if (cm==-1)
	{
		if (c==0)
		{
			cm = pmap[y][x]&0xFF;
			if (!cm)
				return 0;
			if (REPLACE_MODE && cm!=SLALT)
				return 0;
		}
		else
			cm = 0;
	}
	if (bm==-1)
	{
		if (wall==WL_ERASE)
		{
			bm = bmap[y/CELL][x/CELL];
			if (!bm)
				return 0;
			if (bm==WL_WALL)
				cm = 0xFF;
		}
		else
			bm = 0;
	}

	if (((pmap[y][x]&0xFF)!=cm || bmap[y/CELL][x/CELL]!=bm )||( (sdl_mod & (KMOD_CAPS)) && cm!=SLALT))
		return 1;

	// go left as far as possible
	x1 = x2 = x;
	while (x1>=CELL)
	{
		if ((pmap[y][x1-1]&0xFF)!=cm || bmap[y/CELL][(x1-1)/CELL]!=bm)
		{
			break;
		}
		x1--;
	}
	while (x2<XRES-CELL)
	{
		if ((pmap[y][x2+1]&0xFF)!=cm || bmap[y/CELL][(x2+1)/CELL]!=bm)
		{
			break;
		}
		x2++;
	}

	// fill span
	for (x=x1; x<=x2; x++)
	{
		if (!create_parts(x, y, 0, 0, co))
			return 0;
	}
	// fill children
	if (cm==PT_INST&&co==PT_SPRK)//wire crossing for INST
	{
		if (y>=CELL+dy && x1==x2 &&
		        ((pmap[y-1][x1-1]&0xFF)==PT_INST||(pmap[y-1][x1-1]&0xFF)==PT_SPRK) && ((pmap[y-1][x1]&0xFF)==PT_INST||(pmap[y-1][x1]&0xFF)==PT_SPRK) && ((pmap[y-1][x1+1]&0xFF)==PT_INST || (pmap[y-1][x1+1]&0xFF)==PT_SPRK) &&
		        (pmap[y-2][x1-1]&0xFF)!=PT_INST && ((pmap[y-2][x1]&0xFF)==PT_INST ||(pmap[y-2][x1]&0xFF)==PT_SPRK) && (pmap[y-2][x1+1]&0xFF)!=PT_INST)
			flood_parts(x1, y-2, co, cm, bm);
		else if (y>=CELL+dy)
			for (x=x1; x<=x2; x++)
				if ((pmap[y-1][x]&0xFF)!=PT_SPRK)
				{
					if (x==x1 || x==x2 || y>=YRES-CELL-1 ||
					        (pmap[y-1][x-1]&0xFF)==PT_INST || (pmap[y-1][x+1]&0xFF)==PT_INST ||
					        (pmap[y+1][x-1]&0xFF)==PT_INST || ((pmap[y+1][x]&0xFF)!=PT_INST&&(pmap[y+1][x]&0xFF)!=PT_SPRK) || (pmap[y+1][x+1]&0xFF)==PT_INST)
						flood_parts(x, y-dy, co, cm, bm);

				}

		if (y<YRES-CELL-dy && x1==x2 &&
		        ((pmap[y+1][x1-1]&0xFF)==PT_INST||(pmap[y+1][x1-1]&0xFF)==PT_SPRK) && ((pmap[y+1][x1]&0xFF)==PT_INST||(pmap[y+1][x1]&0xFF)==PT_SPRK) && ((pmap[y+1][x1+1]&0xFF)==PT_INST || (pmap[y+1][x1+1]&0xFF)==PT_SPRK) &&
		        (pmap[y+2][x1-1]&0xFF)!=PT_INST && ((pmap[y+2][x1]&0xFF)==PT_INST ||(pmap[y+2][x1]&0xFF)==PT_SPRK) && (pmap[y+2][x1+1]&0xFF)!=PT_INST)
			flood_parts(x1, y+2, co, cm, bm);
		else if (y<YRES-CELL-dy)
			for (x=x1; x<=x2; x++)
				if ((pmap[y+1][x]&0xFF)!=PT_SPRK)
				{
					if (x==x1 || x==x2 || y<0 ||
					        (pmap[y+1][x-1]&0xFF)==PT_INST || (pmap[y+1][x+1]&0xFF)==PT_INST ||
					        (pmap[y-1][x-1]&0xFF)==PT_INST || ((pmap[y-1][x]&0xFF)!=PT_INST&&(pmap[y-1][x]&0xFF)!=PT_SPRK) || (pmap[y-1][x+1]&0xFF)==PT_INST)
						flood_parts(x, y+dy, co, cm, bm);

				}
	}
	else
	{
		if (y>=CELL+dy)
			for (x=x1; x<=x2; x++)
				if ((pmap[y-dy][x]&0xFF)==cm && bmap[(y-dy)/CELL][x/CELL]==bm)
					if (!flood_parts(x, y-dy, co, cm, bm))
						return 0;
		if (y<YRES-CELL-dy)
			for (x=x1; x<=x2; x++)
				if ((pmap[y+dy][x]&0xFF)==cm && bmap[(y+dy)/CELL][x/CELL]==bm)
					if (!flood_parts(x, y+dy, co, cm, bm))
						return 0;
	}
	if (!(cm==PT_INST&&co==PT_SPRK))
		return 1;
}

int create_parts(int x, int y, int rx, int ry, int c)
{
	int i, j, r, f = 0, u, v, oy, ox, b = 0, dw = 0, stemp = 0;//n;

	int wall = c - 100;
	for (r=UI_ACTUALSTART; r<=UI_ACTUALSTART+UI_WALLCOUNT; r++)
	{
		if (wall==r)
		{
			if (c == SPC_AIR || c == SPC_HEAT || c == SPC_COOL || c == SPC_VACUUM)
				break;
			if (wall == WL_ERASE)
				b = 0;
			else
				b = wall;
			dw = 1;
		}
	}
	if (c == WL_FANHELPER)
	{
		b = WL_FANHELPER;
		dw = 1;
	}
	if (dw==1)
	{
		rx = rx/CELL;
		x = x/CELL;
		y = y/CELL;
		x -= rx/2;
		y -= rx/2;
		for (ox=x; ox<=x+rx; ox++)
		{
			for (oy=y; oy<=y+rx; oy++)
			{
				if (ox>=0&&ox<XRES/CELL&&oy>=0&&oy<YRES/CELL)
				{
					i = ox;
					j = oy;
					if (((sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))|| sdl_mod & (KMOD_CAPS) ))
					{
						if (bmap[j][i]==SLALT-100)
							b = 0;
						else
							continue;
					}
					if (b==WL_FAN)
					{
						fvx[j][i] = 0.0f;
						fvy[j][i] = 0.0f;
					}
					if (b==WL_STREAM)
					{
						i = x + rx/2;
						j = y + rx/2;
						for (v=-1; v<2; v++)
							for (u=-1; u<2; u++)
								if (i+u>=0 && i+u<XRES/CELL &&
								        j+v>=0 && j+v<YRES/CELL &&
								        bmap[j+v][i+u] == WL_STREAM)
									return 1;
						bmap[j][i] = WL_STREAM;
						continue;
					}
					bmap[j][i] = b;
				}
			}
		}
		return 1;
	}

	if (((sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))|| sdl_mod & (KMOD_CAPS) )&& !REPLACE_MODE)
	{
		if (rx==0&&ry==0)
		{
			delete_part(x, y);
		}
		else
			for (j=-ry; j<=ry; j++)
				for (i=-rx; i<=rx; i++)
					if ((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
						delete_part(x+i, y+j);
		return 1;
	}

	if (c == SPC_AIR || c == SPC_HEAT || c == SPC_COOL || c == SPC_VACUUM)
	{
		if (rx==0&&ry==0)
		{
			create_part(-2, x, y, c);
		}
		else
			for (j=-ry; j<=ry; j++)
				for (i=-rx; i<=rx; i++)
					if ((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
						if (!REPLACE_MODE)
							create_part(-2, x+i, y+j, c);
						else if ((pmap[y+j][x+i]&0xFF)==SLALT&&SLALT!=0)
							create_part(-2, x+i, y+j, c);
		return 1;
	}

	if (c == 0 && !REPLACE_MODE)
	{
		stemp = SLALT;
		SLALT = 0;
		if (rx==0&&ry==0)
		{
			delete_part(x, y);
		}
		else
			for (j=-ry; j<=ry; j++)
				for (i=-rx; i<=rx; i++)
					if ((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
						delete_part(x+i, y+j);
		SLALT = stemp;
		return 1;
	}
	if (REPLACE_MODE)
	{
		if (rx==0&&ry==0)
		{
			if ((pmap[y][x]&0xFF)==SLALT || SLALT==0)
			{
				if ((pmap[y][x]))
				{
					delete_part(x, y);
					if (c!=0)
						create_part(-2, x, y, c);
				}
			}
		}
		else
			for (j=-ry; j<=ry; j++)
				for (i=-rx; i<=rx; i++)
					if ((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
					{
						if ((pmap[y+j][x+i]&0xFF)!=SLALT&&SLALT!=0)
							continue;
						if ((pmap[y+j][x+i]))
						{
							delete_part(x+i, y+j);
							if (c!=0)
								create_part(-2, x+i, y+j, c);
						}
					}
		return 1;

	}
	if (rx==0&&ry==0)//workaround for 1pixel brush/floodfill crashing. todo: find a better fix later.
	{
		if (create_part(-2, x, y, c)==-1)
			f = 1;
	}
	else
		for (j=-ry; j<=ry; j++)
			for (i=-rx; i<=rx; i++)
				if ((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
					if (create_part(-2, x+i, y+j, c)==-1)
						f = 1;
	return !f;
}

void create_line(int x1, int y1, int x2, int y2, int rx, int ry, int c)
{
	int cp=abs(y2-y1)>abs(x2-x1), x, y, dx, dy, sy;
	float e, de;
	if (cp)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
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
	if (dx)
		de = dy/(float)dx;
	else
		de = 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (cp)
			create_parts(y, x, rx, ry, c);
		else
			create_parts(x, y, rx, ry, c);
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (c==WL_EHOLE || c==WL_ALLOWGAS || c==WL_ALLOWALLELEC || c==WL_ALLOWSOLID || c==WL_ALLOWAIR || c==WL_WALL || c==WL_DESTROYALL || c==WL_ALLOWLIQUID || c==WL_FAN || c==WL_STREAM || c==WL_DETECT || c==WL_EWALL || c==WL_WALLELEC || !(rx+ry))
			{
				if (cp)
					create_parts(y, x, rx, ry, c);
				else
					create_parts(x, y, rx, ry, c);
			}
			e -= 1.0f;
		}
	}
}
