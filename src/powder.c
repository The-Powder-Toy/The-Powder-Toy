#include <stdint.h>
#include <math.h>
#include <defines.h>
#include <powder.h>
#include <air.h>
#include <misc.h>

int gravwl_timeout = 0;

int isplayer = 0;
float player[27]; //[0] is a command cell, [3]-[18] are legs positions, [19] is index, [19]-[26] are accelerations
float player2[27];

particle *parts;
particle *cb_parts;

int gravityMode = 0; // starts enabled in "vertical" mode...
int airMode = 0;


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

static void photoelectric_effect(int nx, int ny)//create sparks from PHOT when hitting PSCN and NSCN
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

unsigned char can_move[PT_NUM][PT_NUM];

void init_can_move()
{
	// can_move[moving type][type at destination]
	//  0 = No move/Bounce
	//  1 = Swap
	//  2 = Both particles occupy the same space.
	//  3 = Varies, go run some extra checks
	int t, rt;
	for (rt=0;rt<PT_NUM;rt++)
		can_move[0][rt] = 0; // particles that don't exist shouldn't move...
	for (t=1;t<PT_NUM;t++)
		for (rt=0;rt<PT_NUM;rt++)
			can_move[t][rt] = 1;
	for (rt=1;rt<PT_NUM;rt++)
	{
		can_move[PT_PHOT][rt] = 2;
	}
	for (t=1;t<PT_NUM;t++)
	{
		for (rt=1;rt<PT_NUM;rt++)
		{
			// weight check, also prevents particles of same type displacing each other
			if (ptypes[t].weight <= ptypes[rt].weight) can_move[t][rt] = 0;
			if (t==PT_NEUT && ptypes[rt].properties&PROP_NEUTPASS)
				can_move[t][rt] = 2;
			if (t==PT_NEUT && ptypes[rt].properties&PROP_NEUTPENETRATE)
				can_move[t][rt] = 1;
			if (ptypes[t].properties&PROP_NEUTPENETRATE && rt==PT_NEUT)
				can_move[t][rt] = 0;
			if (ptypes[t].properties&TYPE_ENERGY && ptypes[rt].properties&TYPE_ENERGY)
				can_move[t][rt] = 2;
		}
	}
	can_move[PT_BIZR][PT_FILT] = 2;
	can_move[PT_BIZRG][PT_FILT] = 2;
	for (t=0;t<PT_NUM;t++)
	{
		//spark shouldn't move
		can_move[PT_SPRK][t] = 0;
		//all stickman collisions are done in stickman update function
		can_move[PT_STKM][t] = 2;
		can_move[PT_STKM2][t] = 2;
	}
	for (t=0;t<PT_NUM;t++)
	{
		// make them eat things
		can_move[t][PT_VOID] = 1;
		can_move[t][PT_BHOL] = 1;
		can_move[t][PT_NBHL] = 1;
		//all stickman collisions are done in stickman update function
		can_move[t][PT_STKM] = 2;
		can_move[t][PT_STKM2] = 2;
		//INVIS behaviour varies with pressure
		can_move[t][PT_INVIS] = 3;
		//stop CNCT being displaced by other particles
		can_move[t][PT_CNCT] = 0;
	}
	for (t=0;t<PT_NUM;t++)
	{
		if (t==PT_GLAS || t==PT_PHOT || t==PT_CLNE || t==PT_PCLN
			|| t==PT_GLOW || t==PT_WATR || t==PT_DSTW || t==PT_SLTW
			|| t==PT_ISOZ || t==PT_ISZS || t==PT_FILT || t==PT_INVIS
			|| t==PT_QRTZ || t==PT_PQRT)
			can_move[PT_PHOT][t] = 2;
	}
	can_move[PT_PHOT][PT_LCRY] = 3;//varies according to LCRY life
	can_move[PT_NEUT][PT_INVIS] = 2;
	//whol eats anar
	can_move[PT_ANAR][PT_WHOL] = 2;
	can_move[PT_ANAR][PT_NWHL] = 2;
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
	int result;

	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 0;

	r = pmap[ny][nx];
	if (r && (r>>8)<NPART)
		r = (r&~0xFF) | parts[r>>8].type;
	if (rr)
		*rr = r;
	if (pt>=PT_NUM || (r&0xFF)>=PT_NUM)
		return 0;
	result = can_move[pt][r&0xFF];
	if (result==3)
	{
		if (pt==PT_PHOT && (r&0xFF)==PT_LCRY)
			result = (parts[r>>8].life > 5)? 2 : 0;
		if ((r&0xFF)==PT_INVIS)
		{
			if (pv[ny/CELL][nx/CELL]>4.0f || pv[ny/CELL][nx/CELL]<-4.0f) result = 2;
			else result = 0;
		}
	}
	if (bmap[ny/CELL][nx/CELL])
	{
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWGAS && ptypes[pt].falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWLIQUID && ptypes[pt].falldown!=2)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWSOLID && ptypes[pt].falldown!=1)
			return 0;
		// blocking by WL_WALL, WL_WALLELEC and unpowered WL_EWALL is currently done by putting 0x7FFFFFFF in pmap
		if (bmap[ny/CELL][nx/CELL]==WL_ALLOWAIR)
			return 0;
		if (bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL])
			return 2;
	}
	return result;
}

int try_move(int i, int x, int y, int nx, int ny)
{
	unsigned r, e;

	if (x==nx && y==ny)
		return 1;
	if (nx<0 || ny<0 || nx>=XRES || ny>=YRES)
		return 1;

	e = eval_move(parts[i].type, nx, ny, &r);

	if ((r&0xFF)==PT_BOMB && parts[i].type==PT_BOMB && parts[i].tmp == 1)
		e = 2;

	/* half-silvered mirror */
	if (!e && parts[i].type==PT_PHOT &&
	        (((r&0xFF)==PT_BMTL && rand()<RAND_MAX/2) ||
	         (pmap[y][x]&0xFF)==PT_BMTL))
		e = 2;

	if (!e) //if no movement
	{
		if (parts[i].type!=PT_NEUT && parts[i].type!=PT_PHOT)
			return 0;
		if (!legacy_enable && parts[i].type==PT_PHOT && r)//PHOT heat conduction
		{
			if ((r & 0xFF) == PT_COAL || (r & 0xFF) == PT_BCOL)
				parts[r>>8].temp = parts[i].temp;

			if ((r & 0xFF) < PT_NUM && ptypes[r&0xFF].hconduct)
				parts[i].temp = parts[r>>8].temp = restrict_flt((parts[r>>8].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}
		if (parts[i].type==PT_NEUT && ((r&0xFF)==PT_CLNE || (r&0xFF)==PT_PCLN || (r&0xFF)==PT_BCLN)) {
			if (!parts[r>>8].ctype)
				parts[r>>8].ctype = PT_NEUT;
		}
		if ((r&0xFF)==PT_PRTI && (parts[i].type==PT_PHOT || parts[i].type==PT_NEUT))
		{
			int nnx, count;
			if (nx-x<0)
			{
				if (ny-y<0) count = 1;
				else if (ny-y==0) count = 2;
				else count = 3;
			}
			else if (nx-x==0)
			{
				if (ny-y<0) count = 4;
				else count = 5;
			}
			else
			{
				if (ny-y<0) count = 6;
				else if (ny-y==0) count = 7;
				else count = 8;
			}
			parts[r>>8].tmp = (int)((parts[r>>8].temp-73.15f)/100+1);
			if (parts[r>>8].tmp>=CHANNELS) parts[r>>8].tmp = CHANNELS-1;
			else if (parts[r>>8].tmp<0) parts[r>>8].tmp = 0;
			for ( nnx=0; nnx<80; nnx++)
				if (!portalp[parts[r>>8].tmp][count-1][nnx].type)
				{
					portalp[parts[r>>8].tmp][count-1][nnx] = parts[i];
					parts[i].type=PT_NONE;
					break;
				}
		}
		return 0;
	}

	if (e == 2) //if occupy same space
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
		if (parts[i].type == PT_PHOT && (r&0xFF)==PT_INVIS && pv[ny/CELL][nx/CELL]<=4.0f && pv[ny/CELL][nx/CELL]>=-4.0f) {
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
	//else e=1 , we are trying to swap the particles, return 0 no swap/move, 1 is still overlap/move, because the swap takes place later

	if ((r&0xFF)==PT_VOID) //this is where void eats particles
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
	if ((r&0xFF)==PT_BHOL || (r&0xFF)==PT_NBHL) //this is where blackhole eats particles
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
	if (((r&0xFF)==PT_WHOL||(r&0xFF)==PT_NWHL) && parts[i].type==PT_ANAR) //whitehole eats anar
	{
		parts[i].type=PT_NONE;
		if (!legacy_enable)
		{
			parts[r>>8].temp = restrict_flt(parts[r>>8].temp- (MAX_TEMP-parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
		}

		return 0;
	}

	if (parts[i].type==PT_CNCT && y<ny && (pmap[y+1][x]&0xFF)==PT_CNCT)//check below CNCT for another CNCT
		return 0;

	if ((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && !(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL]))
		return 0;

	e = r >> 8; //e is now the particle number at r (pmap[ny][nx])
	if (r && e<NPART)//the swap part, if we make it this far, swap
	{
		if (parts[i].type==PT_NEUT) {
			// target material is NEUTPENETRATE, meaning it gets moved around when neutron passes
			unsigned s = pmap[y][x];
			if ((s>>8)>=NPART) return 0;
			if ((s&0xFF) && (s&0xFF)<PT_NUM && !(ptypes[s&0xFF].properties&PROP_NEUTPENETRATE))
				return 1; // if the element currently underneath neutron isn't NEUTPENETRATE, don't move anything except the neutron
			// if nothing is currently underneath neutron, only move target particle
			if (s)
			{
				pmap[ny][nx] = (s&~(0xFF))|parts[s>>8].type;
				parts[s>>8].x = nx;
				parts[s>>8].y = ny;
			}
			else pmap[ny][nx] = 0;
			parts[e].x = x;
			parts[e].y = y;
			pmap[y][x] = (e<<8)|parts[e].type;
			return 1;
		}

		if ((pmap[ny][nx]>>8)==e) pmap[ny][nx] = 0;
		parts[e].x += x-nx;
		parts[e].y += y-ny;
		pmap[(int)(parts[e].y+0.5f)][(int)(parts[e].x+0.5f)] = (e<<8)|parts[e].type;
	}

	return 1;
}

// try to move particle, and if successful update pmap and parts[i].x,y
int do_move(int i, int x, int y, float nxf, float nyf)
{
	int nx = (int)(nxf+0.5f), ny = (int)(nyf+0.5f);
	int result = try_move(i, x, y, nx, ny);
	if (result)
	{
		int t = parts[i].type;
		parts[i].x = nxf;
		parts[i].y = nyf;
		if (ny!=y || nx!=x)
		{
			if ((pmap[y][x]>>8)==i) pmap[y][x] = 0;
			else if ((photons[y][x]>>8)==i) photons[y][x] = 0;
			if (nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)//kill_part if particle is out of bounds
			{
				kill_part(i);
				return -1;
			}
			if (t==PT_PHOT||t==PT_NEUT)
				photons[ny][nx] = t|(i<<8);
			else if (t)
				pmap[ny][nx] = t|(i<<8);
		}
	}
	return result;
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

//For soap only
void detach(int i)
{
	if ((parts[i].ctype&2) == 2)
	{
		if ((parts[parts[i].tmp].ctype&4) == 4)
			parts[parts[i].tmp].ctype ^= 4;
	}

	if ((parts[i].ctype&4) == 4)
	{
		if ((parts[parts[i].tmp2].ctype&2) == 2)
			parts[parts[i].tmp2].ctype ^= 2;
	}

	parts[i].ctype = 0;
}

void kill_part(int i)//kills particle number i
{
	int x, y;

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
	if (parts[i].type == PT_SOAP)
	{
		detach(i);
	}
	if (x>=0 && y>=0 && x<XRES && y<YRES) {
		if ((pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
		else if ((photons[y][x]>>8)==i)
			photons[y][x] = 0;
	}

	parts[i].type = PT_NONE;
	parts[i].life = pfree;
	pfree = i;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void part_change_type(int i, int x, int y, int t)
#else
inline void part_change_type(int i, int x, int y, int t)//changes the type of particle number i, to t.  This also changes pmap at the same time.
#endif
{
	if (x<0 || y<0 || x>=XRES || y>=YRES || i>=NPART || t<0 || t>=PT_NUM)
		return;
	if (t==OLD_PT_WIND)
		t = PT_NONE;
	parts[i].type = t;
	if (t==PT_PHOT || t==PT_NEUT)
	{
		photons[y][x] = t|(i<<8);
		if ((pmap[y][x]>>8)==i)
			pmap[y][x] = 0;
	}
	else
	{
		pmap[y][x] = t|(i<<8);
		if ((photons[y][x]>>8)==i)
			photons[y][x] = 0;
	}
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int create_part(int p, int x, int y, int t)
#else
inline int create_part(int p, int x, int y, int t)//the function for creating a particle, use p=-1 for creating a new particle, -2 is from a brush, or a particle number to replace a particle.
#endif
{
	int i;

	if (x<0 || y<0 || x>=XRES || y>=YRES || ((t<0 || t>=PT_NUM)&&t!=SPC_HEAT&&t!=SPC_COOL&&t!=SPC_AIR&&t!=SPC_VACUUM))
		return -1;
	if (t==OLD_PT_WIND)
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
		if ((pmap[y][x]>>8)>=NPART || !((pmap[y][x]&0xFF)==PT_INST||(ptypes[pmap[y][x]&0xFF].properties&PROP_CONDUCTS)))
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
		if (photons[y][x] && (t==PT_PHOT||t==PT_NEUT))
			return -1;
		if (pfree == -1)
			return -1;
		i = pfree;
		pfree = parts[i].life;
	}
	else
		i = p;

	parts[i].dcolour = 0;
	if (t==PT_GLAS)
	{
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
	}
	if (t==PT_QRTZ)
	{
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
	}
	if (t!=PT_STKM&&t!=PT_STKM2)//set everything to default values first, except for stickman.
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
		parts[i].tmp2 = 0;
	}
	if (t==PT_SOAP)
	{
		parts[i].tmp = -1;
		parts[i].tmp2 = -1;
	}
	//now set various properties that we want at spawn.
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
	if (ptypes[t].properties&PROP_LIFE) {
		int r;
		for (r = 0; r<NGOL; r++)
			if (t==goltype[r])
				parts[i].tmp = grule[r+1][9] - 1;
	}
	if (t==PT_DEUT)
		parts[i].life = 10;
	if (t==PT_MERC)
		parts[i].tmp = 10;
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
	if (t==PT_STKM)
	{
		if (isplayer==0)
		{
			parts[i].x = (float)x;
			parts[i].y = (float)y;
			parts[i].type = PT_STKM;
			parts[i].vx = 0;
			parts[i].vy = 0;
			parts[i].life = 100;
			parts[i].ctype = 0;
			parts[i].temp = ptypes[t].heat;

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
		create_part(-1,x,y,PT_SPAWN);
		ISSPAWN1 = 1;
	}
	if (t==PT_STKM2)
	{
		if (isplayer2==0)
		{
			parts[i].x = (float)x;
			parts[i].y = (float)y;
			parts[i].type = PT_STKM2;
			parts[i].vx = 0;
			parts[i].vy = 0;
			parts[i].life = 100;
			parts[i].ctype = 0;
			parts[i].temp = ptypes[t].heat;

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
		create_part(-1,x,y,PT_SPAWN2);
		ISSPAWN2 = 1;
	}
	if (t==PT_BIZR||t==PT_BIZRG)
		parts[i].ctype = 0x47FFFF;
	//and finally set the pmap/photon maps to the newly created particle
	if (t==PT_PHOT||t==PT_NEUT)
		photons[y][x] = t|(i<<8);
	if (t!=PT_STKM&&t!=PT_STKM2 && t!=PT_PHOT && t!=PT_NEUT)
		pmap[y][x] = t|(i<<8);

	return i;
}

static void create_gain_photon(int pp)//photons from PHOT going through GLOW
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

static void create_cherenkov_photon(int pp)//photons from NEUT going through GLAS
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
inline void delete_part(int x, int y)//calls kill_part with the particle located at x,y
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
	if ((parts[i>>8].type==SLALT)||SLALT==0)//specific deletiom
	{
		kill_part(i>>8);
	}
	else if (ptypes[parts[i>>8].type].menusection==SEC)//specific menu deletion
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
_inline int parts_avg(int ci, int ni,int t)//t is the particle you are looking for, returns the particle between two particles
#else
inline int parts_avg(int ci, int ni,int t)
#endif
{
	if (t==PT_INSL)//to keep electronics working
	{
		int pmr = pmap[((int)(parts[ci].y+0.5f) + (int)(parts[ni].y+0.5f))/2][((int)(parts[ci].x+0.5f) + (int)(parts[ni].x+0.5f))/2];
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
			ndistance = abs(cx-parts[i].x)+abs(cy-parts[i].y);// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
			if (ndistance<distance)
			{
				distance = ndistance;
				id = i;
			}
		}
	}
	return id;
}

//the main function for updating particles
void update_particles_i(pixel *vid, int start, int inc)
{
	int i, j, x, y, t, nx, ny, r, surround_space, s, lt, rt, nt, nnx, nny, q, golnum, goldelete, z, neighbors, createdsomething;
	float mv, dx, dy, ix, iy, lx, ly, nrx, nry, dp, ctemph, ctempl, gravtot;
	int fin_x, fin_y, clear_x, clear_y;
	float fin_xf, fin_yf, clear_xf, clear_yf;
	float nn, ct1, ct2, swappage;
	float pt = R_TEMP;
	float c_heat = 0.0f;
	int h_count = 0;
	int starti = (start*-1);
	int surround[8];
	int surround_hconduct[8];
	float pGravX, pGravY, pGravD;

	if (sys_pause&&!framerender)//do nothing if paused
		return;
	if (ISGRAV==1)//crappy grav color handling, i will change this someday
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
	if (ISLOVE==1)//LOVE element handling
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
	if (ISLOLZ==1)//LOLZ element handling
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
	//game of life!
	if (ISGOL==1&&++CGOL>=GSPEED)//GSPEED is frames per generation
	{
		int createdsomething = 0;
		CGOL=0;
		ISGOL=0;
		for (nx=CELL; nx<XRES-CELL; nx++)//go through every particle and set neighbor map
			for (ny=CELL; ny<YRES-CELL; ny++)
			{
				r = pmap[ny][nx];
				if ((r>>8)>=NPART || !r)
				{
					gol[nx][ny] = 0;
					continue;
				}
				else
					for ( golnum=1; golnum<=NGOL; golnum++)
						if (parts[r>>8].type==goltype[golnum-1])
						{
							if (parts[r>>8].tmp == grule[golnum][9]-1) {
								gol[nx][ny] = golnum;
								for ( nnx=-1; nnx<2; nnx++)
									for ( nny=-1; nny<2; nny++)//it will count itself as its own neighbor, which is needed, but will have 1 extra for delete check
									{
										rt = pmap[((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL][((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL];
										if (!rt || ptypes[rt&0xFF].properties&PROP_LIFE)
										{
											gol2[((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL][((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL][golnum] ++;
											gol2[((nx+nnx+XRES-3*CELL)%(XRES-2*CELL))+CELL][((ny+nny+YRES-3*CELL)%(YRES-2*CELL))+CELL][0] ++;
										}
									}
							} else {
								parts[r>>8].tmp --;
								if (parts[r>>8].tmp<=0)
									parts[r>>8].type = PT_NONE;//using kill_part makes it not work
							}
						}
			}
		for (nx=CELL; nx<XRES-CELL; nx++)//go through every particle again, but check neighbor map, then update particles
			for (ny=CELL; ny<YRES-CELL; ny++)
			{
				r = pmap[ny][nx];
				neighbors = gol2[nx][ny][0];
				if (neighbors==0 || !(ptypes[r&0xFF].properties&PROP_LIFE || !(r&0xFF)) || (r>>8)>=NPART)
					continue;
				for ( golnum = 1; golnum<=NGOL; golnum++)
				{
					goldelete = neighbors;
					if (gol[nx][ny]==0&&grule[golnum][goldelete]>=2&&gol2[nx][ny][golnum]>=(goldelete%2)+goldelete/2)
					{
						if (create_part(-1,nx,ny,goltype[golnum-1]))
							createdsomething = 1;
					}
					else if (gol[nx][ny]==golnum&&(grule[golnum][goldelete-1]==0||grule[golnum][goldelete-1]==2))//subtract 1 because it counted itself
					{
						if (parts[r>>8].tmp==grule[golnum][9]-1)
							parts[r>>8].tmp --;
					}
					if (r && parts[r>>8].tmp<=0)
						parts[r>>8].type = PT_NONE;//using kill_part makes it not work
				}
				for ( z = 0; z<=NGOL; z++)
					gol2[nx][ny][z] = 0;//this improves performance A LOT compared to the memset, i was getting ~23 more fps with this.
			}
		if (createdsomething)
			GENERATION ++;
		//memset(gol2, 0, sizeof(gol2));
	}
	if (ISWIRE==1)//wifi channel reseting
	{
		for ( q = 0; q<(int)(MAX_TEMP-73.15f)/100+2; q++)
			if (!wireless[q][1])
			{
				wireless[q][0] = 0;
			}
			else
				wireless[q][1] = 0;
	}
	//the main particle loop function, goes over all particles.
	for (i=start; i<(NPART-starti); i+=inc)
		if (parts[i].type)
		{
			lx = parts[i].x;
			ly = parts[i].y;
			t = parts[i].type;
			if (t<0 || t>=PT_NUM)
			{
				kill_part(i);
				continue;
			}
			//printf("parts[%d].type: %d\n", i, parts[i].type);

			if (parts[i].life>0 && (ptypes[t].properties&PROP_LIFE_DEC))
			{
				// automatically decrease life
				parts[i].life--;
				if (parts[i].life<=0 && (ptypes[t].properties&(PROP_LIFE_KILL_DEC|PROP_LIFE_KILL)))
				{
					// kill on change to no life
					kill_part(i);
					continue;
				}
			}
			else if (parts[i].life<=0 && (ptypes[t].properties&PROP_LIFE_KILL))
			{
				// kill if no life
				kill_part(i);
				continue;
			}

			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);

			//this kills any particle out of the screen, or in a wall where it isn't supposed to go
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

			//adding to velocity from the particle's velocity
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
			else//add the hotair variable to the pressure map, like black hole, or white hole.
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
				pGravX = 0.0f;
				pGravY = ptypes[t].gravity;
				break;
			case 1:
				pGravX = pGravY = 0.0f;
				break;
			case 2:
				pGravD = 0.01f - hypotf((x - XCNTR), (y - YCNTR));
				pGravX = ptypes[t].gravity * ((float)(x - XCNTR) / pGravD);
				pGravY = ptypes[t].gravity * ((float)(y - YCNTR) / pGravD);
			}
			//Get some gravity from the gravity map
			if (t==PT_ANAR)
			{
				// perhaps we should have a ptypes variable for this
				pGravX -= gravx[y/CELL][x/CELL];
				pGravY -= gravy[y/CELL][x/CELL];
			}
			else if(!(ptypes[t].properties & TYPE_SOLID))
			{
				pGravX += gravx[y/CELL][x/CELL];
				pGravY += gravy[y/CELL][x/CELL];
			}
			//velocity updates for the particle
			parts[i].vx *= ptypes[t].loss;
			parts[i].vy *= ptypes[t].loss;
			//particle gets velocity from the vx and vy maps
			parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL] + pGravX;
			parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL] + pGravY;


			if (ptypes[t].diffusion)//the random diffusion that gasses have
			{
				parts[i].vx += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
				parts[i].vy += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
			}

			j = surround_space = nt = 0;//if nt is 1 after this, then there is a particle around the current particle, that is NOT the current particle's type, for water movement.
			for (nx=-1; nx<2; nx++)
				for (ny=-1; ny<2; ny++) {
					if (nx||ny) {
						surround[j] = r = pmap[y+ny][x+nx];
						j++;
						if (!(r&0xFF))
							surround_space = 1;//there is empty space
						if ((r&0xFF)!=t)
							nt = 1;//there is nothing or a different particle
					}
				}

			if (!legacy_enable)
			{
				if (y-2 >= 0 && y-2 < YRES && (ptypes[t].properties&TYPE_LIQUID)) {//some heat convection for liquids
					r = pmap[y-2][x];
					if (!((r>>8)>=NPART || !r || parts[i].type != (r&0xFF))) {
						if (parts[i].temp>parts[r>>8].temp) {
							swappage = parts[i].temp;
							parts[i].temp = parts[r>>8].temp;
							parts[r>>8].temp = swappage;
						}
					}
				}

				//heat transfer code
				h_count = 0;
				if (t&&(t!=PT_HSWC||parts[i].life==10)&&ptypes[t].hconduct>(rand()%250))
				{
					if (aheat_enable)
					{
						c_heat = (hv[y/CELL][x/CELL]-parts[i].temp)*0.04;
						parts[i].temp += c_heat;
						hv[y/CELL][x/CELL] -= c_heat;
					}
					c_heat = 0.0f;
					float c_Cm = 0.0f;
					for (j=0; j<8; j++)
					{
						surround_hconduct[j] = i;
						r = surround[j];
						if ((r>>8)>=NPART || !r)
							continue;
						rt = r&0xFF;
						if (rt&&ptypes[rt].hconduct&&ptypes[rt].weight>0&&(rt!=PT_HSWC||parts[r>>8].life==10)
						        &&(t!=PT_FILT||(rt!=PT_BRAY&&rt!=PT_BIZR&&rt!=PT_BIZRG))
						        &&(rt!=PT_FILT||(t!=PT_BRAY&&t!=PT_PHOT&&t!=PT_BIZR&&t!=PT_BIZRG)))
						{
							surround_hconduct[j] = r>>8;
							c_heat += parts[r>>8].temp*96.645/ptypes[rt].hconduct*ptypes[rt].weight;
							c_Cm += 96.645/ptypes[rt].hconduct*ptypes[rt].weight;
							h_count++;
						}
					}

					pt = parts[i].temp = (c_heat+parts[i].temp*96.645/ptypes[t].hconduct*ptypes[t].weight)/(c_Cm+96.645/ptypes[t].hconduct*ptypes[t].weight);
					for (j=0; j<8; j++)
					{
						parts[surround_hconduct[j]].temp = pt;
					}

					ctemph = ctempl = pt;
					// change boiling point with pressure
					if ((ptypes[t].state==ST_LIQUID && ptransitions[t].tht>-1 && ptransitions[t].tht<PT_NUM && ptypes[ptransitions[t].tht].state==ST_GAS)
					        || t==PT_LNTG || t==PT_SLTW)
						ctemph -= 2.0f*pv[y/CELL][x/CELL];
					else if ((ptypes[t].state==ST_GAS && ptransitions[t].tlt>-1 && ptransitions[t].tlt<PT_NUM && ptypes[ptransitions[t].tlt].state==ST_LIQUID)
					         || t==PT_WTRV)
						ctempl -= 2.0f*pv[y/CELL][x/CELL];
					s = 1;
					if (ctemph>ptransitions[t].thv&&ptransitions[t].tht>-1) {
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
					} else if (ctempl<ptransitions[t].tlv&&ptransitions[t].tlt>-1) {
						// particle type change due to low temperature
						if (ptransitions[t].tlt!=PT_NUM)
							t = ptransitions[t].tlt;
						else if (t==PT_WTRV) {
							if (pt<273.0f) t = PT_RIME;
							else t = PT_DSTW;
						}
						else if (t==PT_LAVA) {
							if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && parts[i].ctype!=PT_LAVA) {
								if (parts[i].ctype==PT_THRM&&pt>=ptransitions[PT_BMTL].thv) s = 0;
								else if (ptransitions[parts[i].ctype].tht==PT_LAVA) {
									if (pt>=ptransitions[parts[i].ctype].thv) s = 0;
								}
								else if (pt>=973.0f) s = 0; // freezing point for lava with any other (not listed in ptransitions as turning into lava) ctype
								if (s) {
									t = parts[i].ctype;
									parts[i].ctype = PT_NONE;
									if (t==PT_THRM) {
										parts[i].tmp = 0;
										t = PT_BMTL;
									}
									if (t==PT_PLUT)
									{
										parts[i].tmp = 0;
										t = PT_LAVA;
									}
								}
							}
							else if (pt<973.0f) t = PT_STNE;
							else s = 0;
						}
						else s = 0;
					}
					else s = 0;
					if (s) { // particle type change occurred
						if (t==PT_ICEI||t==PT_LAVA)
							parts[i].ctype = parts[i].type;
						if (!(t==PT_ICEI&&parts[i].ctype==PT_FRZW)) parts[i].life = 0;
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
				ISGOL=1;//means there is a life particle on screen
			}
			//spark updates from walls
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

			//the basic explosion, from the .explosive variable
			if ((ptypes[t].explosive&2) && pv[y/CELL][x/CELL]>2.5f)
			{
				parts[i].life = rand()%80+180;
				parts[i].temp = restrict_flt(ptypes[PT_FIRE].heat + (ptypes[t].flammable/2), MIN_TEMP, MAX_TEMP);
				t = PT_FIRE;
				part_change_type(i,x,y,t);
				pv[y/CELL][x/CELL] += 0.25f * CFDS;
			}


			s = 1;
			gravtot = fabsf(gravy[y/CELL][x/CELL])+fabsf(gravx[y/CELL][x/CELL]);
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
			} else if (gravtot>(ptransitions[t].phv/4.0f)&&ptransitions[t].pht>-1) {
				// particle type change due to high gravity
				if (ptransitions[t].pht!=PT_NUM)
					t = ptransitions[t].pht;
				else if (t==PT_BMTL) {
					if (gravtot>0.625f)
						t = PT_BRMT;
					else if (gravtot>0.25f && parts[i].tmp==1)
						t = PT_BRMT;
					else s = 0;
				}
				else s = 0;
			} else s = 0;
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

			//call the particle update function, if there is one
			if (ptypes[t].update_func)
			{
				if ((*(ptypes[t].update_func))(i,x,y,surround_space))
					continue;
			}
			if (legacy_enable)//if heat sim is off
				update_legacy_all(i,x,y,surround_space);

killed:
			if (parts[i].type == PT_NONE)//if its dead, skip to next particle
				continue;

			if (!parts[i].vx&&!parts[i].vy)//if its not moving, skip to next particle, movement code it next
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
					if (fin_x<CELL || fin_y<CELL || fin_x>=XRES-CELL || fin_y>=YRES-CELL || pmap[fin_y][fin_x] || (bmap[fin_y/CELL][fin_x/CELL] && (bmap[fin_y/CELL][fin_x/CELL]==WL_DESTROYALL || bmap[fin_y/CELL][fin_x/CELL]==WL_DETECT || !eval_move(t,fin_x,fin_y,NULL))))
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

			rt = parts[i].flags & FLAG_STAGNANT;
			parts[i].flags &= ~FLAG_STAGNANT;

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
				if (!do_move(i, x, y, fin_xf, fin_yf))
				{
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
						if (t==PT_PHOT) {
							s = eval_move(PT_PHOT, fin_x, fin_y, &r);
							if ((((r&0xFF)==PT_GLAS && lt!=PT_GLAS) || ((r&0xFF)!=PT_GLAS && lt==PT_GLAS)) && s) {
								// if movement will involve refraction, leave it until next frame so that refraction happens correctly
								goto movedone;
							}
						}
						// cast coords as int then back to float for compatibility with existing saves
						if (!do_move(i, x, y, (float)fin_x, (float)fin_y)) {
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
				if (!do_move(i, x, y, fin_xf, fin_yf))
				{
					// can't move there, so bounce off
					// TODO
					if (fin_x>x+ISTP) fin_x=x+ISTP;
					if (fin_x<x-ISTP) fin_x=x-ISTP;
					if (fin_y>y+ISTP) fin_y=y+ISTP;
					if (fin_y<y-ISTP) fin_y=y-ISTP;
					if (do_move(i, x, y, 0.25f+(float)(2*x-fin_x), 0.25f+fin_y))
					{
						parts[i].vx *= ptypes[t].collision;
					}
					else if (do_move(i, x, y, 0.25f+fin_x, 0.25f+(float)(2*y-fin_y)))
					{
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
				if (!do_move(i, x, y, fin_xf, fin_yf))
				{
					if (fin_x!=x && do_move(i, x, y, fin_xf, clear_yf))
					{
						parts[i].vx *= ptypes[t].collision;
						parts[i].vy *= ptypes[t].collision;
					}
					else if (fin_y!=y && do_move(i, x, y, clear_xf, fin_yf))
					{
						parts[i].vx *= ptypes[t].collision;
						parts[i].vy *= ptypes[t].collision;
					}
					else
					{
						s = 1;
						r = (rand()%2)*2-1;
						if ((clear_x!=x || clear_y!=y || nt || surround_space) &&
							(fabsf(parts[i].vx)>0.01f || fabsf(parts[i].vy)>0.01f))
						{
							// allow diagonal movement if target position is blocked
							// but no point trying this if particle is stuck in a block of identical particles
							dx = parts[i].vx - parts[i].vy*r;
							dy = parts[i].vy + parts[i].vx*r;
							if (fabsf(dy)>fabsf(dx))
								mv = fabsf(dy);
							else
								mv = fabsf(dx);
							dx /= mv;
							dy /= mv;
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= ptypes[t].collision;
								parts[i].vy *= ptypes[t].collision;
								goto movedone;
							}
							swappage = dx;
							dx = dy*r;
							dy = -swappage*r;
							if (do_move(i, x, y, clear_xf+dx, clear_yf+dy))
							{
								parts[i].vx *= ptypes[t].collision;
								parts[i].vy *= ptypes[t].collision;
								goto movedone;
							}
						}
						if (ptypes[t].falldown>1 && (parts[i].vy>fabsf(parts[i].vx) || gravityMode==2))
						{
							// TODO: rewrite to operate better with radial gravity
							s = 0;
							// rt is true if FLAG_STAGNANT was set for this particle in previous frame
							if (!rt || nt) //nt is if there is an something else besides the current particle type, around the particle
								rt = 30;//slight less water lag, although it changes how it moves a lot
							else
								rt = 10;
							for (j=clear_x+r; j>=0 && j>=clear_x-rt && j<clear_x+rt && j<XRES; j+=r)
							{
								if (((pmap[fin_y][j]&0xFF)!=t || bmap[fin_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, fin_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if (fin_y!=clear_y && ((pmap[clear_y][j]&0xFF)!=t || bmap[clear_y/CELL][j/CELL])
									&& (s=do_move(i, x, y, (float)j, clear_yf)))
								{
									nx = (int)(parts[i].x+0.5f);
									ny = (int)(parts[i].y+0.5f);
									break;
								}
								if ((pmap[fin_y][j]&0xFF)!=t || (bmap[fin_y/CELL][j/CELL] && bmap[fin_y/CELL][j/CELL]!=WL_STREAM))
									break;
							}
							if (parts[i].vy>0)
								r = 1;
							else
								r = -1;
							if (s==1)
								for (j=ny+r; j>=0 && j<YRES && j>=ny-rt && j<ny+rt; j+=r)
								{
									if (((pmap[j][nx]&0xFF)!=t || bmap[j/CELL][nx/CELL]) && do_move(i, nx, ny, (float)nx, (float)j))
										break;
									if ((pmap[j][nx]&255)!=t || (bmap[j/CELL][nx/CELL] && bmap[j/CELL][nx/CELL]!=WL_STREAM))
										break;
								}
							else if (s==-1) {} // particle is out of bounds
							else if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {}
							else parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
						else
						{
							// if interpolation was done, try moving to last clear position
							if ((clear_x!=x||clear_y!=y) && do_move(i, x, y, clear_xf, clear_yf)) {}
							else parts[i].flags |= FLAG_STAGNANT;
							parts[i].vx *= ptypes[t].collision;
							parts[i].vy *= ptypes[t].collision;
						}
					}
				}
			}
movedone:
			continue;
		}
	if (framerender) {
		framerender = 0;
		sys_pause = 1;
	}
}

void update_particles(pixel *vid)//doesn't update the particles themselves, but some other things
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
	for (j=0; j<NPART; j++)//the particle loop that resets the pmap/photon maps every frame, to update them.
	{
		i = r ? (NPART-1-j) : j;
		if (parts[i].type)
		{
			t = parts[i].type;
			x = (int)(parts[i].x+0.5f);
			y = (int)(parts[i].y+0.5f);
			if (x>=0 && y>=0 && x<XRES && y<YRES)
			{
				if (t==PT_PHOT||t==PT_NEUT)
					photons[y][x] = t|(i<<8);
				else
					pmap[y][x] = t|(i<<8);
			}
			NUM_PARTS ++;
		}
		else
		{
			parts[i].life = l;
			l = i;
		}
	}
	pfree=l;
	for (y=0; y<YRES/CELL; y++)
	{
		for (x=0; x<XRES/CELL; x++)
		{
			if (bmap[y][x]==WL_WALL || bmap[y][x]==WL_WALLELEC || (bmap[y][x]==WL_EWALL&&!emap[y][x]))
				for (j=0; j<CELL; j++)
					for (i=0; i<CELL; i++)
						pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
			if (emap[y][x] && (!sys_pause||framerender))
				emap[y][x] --;
		}
	}

	update_particles_i(vid, 0, 1);

	// this should probably be elsewhere
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
			create_parts(i, j, 0, 0, c);
}

int flood_parts(int x, int y, int c, int cm, int bm)
{
	int x1, x2, dy = (c<PT_NUM)?1:CELL;
	int co = c;
	if (cm==PT_INST&&co==PT_SPRK)
		if ((pmap[y][x]&0xFF)==PT_SPRK)
			return 0;
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
		if (c-UI_WALLSTART+UI_ACTUALSTART==WL_ERASE)
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

	if (((pmap[y][x]&0xFF)!=cm || bmap[y/CELL][x/CELL]!=bm )||( (sdl_mod & (KMOD_CAPS)) && cm!=SLALT && !(cm==PT_INST&&co==PT_SPRK)))
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
		if (cm==PT_INST&&co==PT_SPRK)
		{
			if (create_part(-1,x, y, co)==-1)
				return 0;
		}
		else if (!create_parts(x, y, 0, 0, co))
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

//this creates particles from a brush, don't use if you want to create one particle
int create_parts(int x, int y, int rx, int ry, int c)
{
	int i, j, r, f = 0, u, v, oy, ox, b = 0, dw = 0, stemp = 0;//n;

	int wall = c - 100;
	if (c==SPC_WIND)
		return 0;
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
	if (wall == WL_GRAV)
	{
		gravwl_timeout = 60;
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
					if (((sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))|| ((sdl_mod & (KMOD_CAPS)) && b!=WL_FANHELPER) ))
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

	//if SHIFT+ALT or CAPSLOCK is on, specific delete
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

	//why do these need a special if
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
					{
						if ( x+i<0 || y+j<0 || x+i>=XRES || y+j>=YRES)
							continue;
						if (!REPLACE_MODE)
							create_part(-2, x+i, y+j, c);
						else if ((pmap[y+j][x+i]&0xFF)==SLALT&&SLALT!=0)
							create_part(-2, x+i, y+j, c);
					}
		return 1;
	}

	//eraser
	if (c == 0 && !REPLACE_MODE)
	{
		stemp = SLALT;
		SLALT = 0;//temporarily clear specific deletion element
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
						if ( x+i<0 || y+j<0 || x+i>=XRES || y+j>=YRES)
							continue;
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
	//else, no special modes, draw element like normal.
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

void *transform_save(void *odata, int *size, matrix2d transform, vector2d translate)
{
	void *ndata;
	unsigned char (*bmapo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	unsigned char (*bmapn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(unsigned char));
	particle *partst = calloc(sizeof(particle), NPART);
	sign *signst = calloc(MAXSIGNS, sizeof(sign));
	unsigned (*pmapt)[XRES] = calloc(YRES*XRES, sizeof(unsigned));
	float (*fvxo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyo)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvxn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	float (*fvyn)[XRES/CELL] = calloc((YRES/CELL)*(XRES/CELL), sizeof(float));
	int i, x, y, nx, ny, w, h, nw, nh;
	vector2d pos, tmp, ctl, cbr;
	vector2d cornerso[4];
	unsigned char *odatac = odata;
	if (parse_save(odata, *size, 0, 0, 0, bmapo, fvxo, fvyo, signst, partst, pmapt))
	{
		free(bmapo);
		free(bmapn);
		free(partst);
		free(signst);
		free(pmapt);
		free(fvxo);
		free(fvyo);
		free(fvxn);
		free(fvyn);
		return odata;
	}
	w = odatac[6]*CELL;
	h = odatac[7]*CELL;
	// undo any translation caused by rotation
	cornerso[0] = v2d_new(0,0);
	cornerso[1] = v2d_new(w-1,0);
	cornerso[2] = v2d_new(0,h-1);
	cornerso[3] = v2d_new(w-1,h-1);
	for (i=0; i<4; i++)
	{
		tmp = m2d_multiply_v2d(transform,cornerso[i]);
		if (i==0) ctl = cbr = tmp; // top left, bottom right corner
		if (tmp.x<ctl.x) ctl.x = tmp.x;
		if (tmp.y<ctl.y) ctl.y = tmp.y;
		if (tmp.x>cbr.x) cbr.x = tmp.x;
		if (tmp.y>cbr.y) cbr.y = tmp.y;
	}
	// casting as int doesn't quite do what we want with negative numbers, so use floor()
	tmp = v2d_new(floor(ctl.x+0.5f),floor(ctl.y+0.5f));
	translate = v2d_sub(translate,tmp);
	nw = floor(cbr.x+0.5f)-floor(ctl.x+0.5f)+1;
	nh = floor(cbr.y+0.5f)-floor(ctl.y+0.5f)+1;
	if (nw>XRES) nw = XRES;
	if (nh>YRES) nh = YRES;
	// rotate and translate signs, parts, walls
	for (i=0; i<MAXSIGNS; i++)
	{
		if (!signst[i].text[0]) continue;
		pos = v2d_new(signst[i].x, signst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			signst[i].text[0] = 0;
			continue;
		}
		signst[i].x = nx;
		signst[i].y = ny;
	}
	for (i=0; i<NPART; i++)
	{
		if (!partst[i].type) continue;
		pos = v2d_new(partst[i].x, partst[i].y);
		pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
		nx = floor(pos.x+0.5f);
		ny = floor(pos.y+0.5f);
		if (nx<0 || nx>=nw || ny<0 || ny>=nh)
		{
			partst[i].type = PT_NONE;
			continue;
		}
		partst[i].x = nx;
		partst[i].y = ny;
	}
	for (y=0; y<YRES/CELL; y++)
		for (x=0; x<XRES/CELL; x++)
		{
			pos = v2d_new(x*CELL+CELL*0.4f, y*CELL+CELL*0.4f);
			pos = v2d_add(m2d_multiply_v2d(transform,pos),translate);
			nx = pos.x/CELL;
			ny = pos.y/CELL;
			if (nx<0 || nx>=nw || ny<0 || ny>=nh)
				continue;
			if (bmapo[y][x])
			{
				bmapn[ny][nx] = bmapo[y][x];
				if (bmapo[y][x]==WL_FAN)
				{
					fvxn[ny][nx] = fvxo[y][x];
					fvyn[ny][nx] = fvyo[y][x];
				}
			}
		}
	ndata = build_save(size,0,0,nw,nh,bmapn,fvxn,fvyn,signst,partst);
	free(bmapo);
	free(bmapn);
	free(partst);
	free(signst);
	free(pmapt);
	free(fvxo);
	free(fvyo);
	free(fvxn);
	free(fvyn);
	return ndata;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[])
#else
inline void orbitalparts_get(int block1, int block2, int resblock1[], int resblock2[])
#endif
{
	resblock1[0] = (block1&0x000000FF);
	resblock1[1] = (block1&0x0000FF00)>>8;
	resblock1[2] = (block1&0x00FF0000)>>16;
	resblock1[3] = (block1&0xFF000000)>>24;

	resblock2[0] = (block2&0x000000FF);
	resblock2[1] = (block2&0x0000FF00)>>8;
	resblock2[2] = (block2&0x00FF0000)>>16;
	resblock2[3] = (block2&0xFF000000)>>24;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[])
#else
inline void orbitalparts_set(int *block1, int *block2, int resblock1[], int resblock2[])
#endif
{
	int block1tmp = 0;
	int block2tmp = 0;

	block1tmp = (resblock1[0]&0xFF);
	block1tmp |= (resblock1[1]&0xFF)<<8;
	block1tmp |= (resblock1[2]&0xFF)<<16;
	block1tmp |= (resblock1[3]&0xFF)<<24;

	block2tmp = (resblock2[0]&0xFF);
	block2tmp |= (resblock2[1]&0xFF)<<8;
	block2tmp |= (resblock2[2]&0xFF)<<16;
	block2tmp |= (resblock2[3]&0xFF)<<24;

	*block1 = block1tmp;
	*block2 = block2tmp;
}
void grav_mask_r(int x, int y, char checkmap[YRES/CELL][XRES/CELL], char shape[YRES/CELL][XRES/CELL], char *shapeout)
{
	if(x < 0 || x >= XRES/CELL || y < 0 || y >= YRES/CELL)
		return;
	if(x == 0 || y ==0 || y == (YRES/CELL)-1 || x == (XRES/CELL)-1)
		*shapeout = 1;
	checkmap[y][x] = 1;
	shape[y][x] = 1;
	if(x-1 >= 0 && !checkmap[y][x-1] && bmap[y][x-1]!=WL_GRAV)
		grav_mask_r(x-1, y, checkmap, shape, shapeout);
	if(y-1 >= 0 && !checkmap[y-1][x] && bmap[y-1][x]!=WL_GRAV)
		grav_mask_r(x, y-1, checkmap, shape, shapeout);
	if(x+1 < XRES/CELL && !checkmap[y][x+1] && bmap[y][x+1]!=WL_GRAV)
		grav_mask_r(x+1, y, checkmap, shape, shapeout);
	if(y+1 < YRES/CELL && !checkmap[y+1][x] && bmap[y+1][x]!=WL_GRAV)
		grav_mask_r(x, y+1, checkmap, shape, shapeout);
	return;
}
struct mask_el {
	char *shape;
	char shapeout;
	void *next;
};
typedef struct mask_el mask_el;
void mask_free(mask_el *c_mask_el){
	if(c_mask_el==NULL)
		return;
	if(c_mask_el->next!=NULL)
		mask_free(c_mask_el->next);
	free(c_mask_el->shape);
	free(c_mask_el);
}
void gravity_mask()
{
	char checkmap[YRES/CELL][XRES/CELL];
	int x = 0, y = 0;
	mask_el *t_mask_el = NULL;
	mask_el *c_mask_el = NULL;
	memset(checkmap, 0, sizeof(checkmap));
	for(x = 0; x < XRES/CELL; x++)
	{
		for(y = 0; y < YRES/CELL; y++)
		{
			if(bmap[y][x]!=WL_GRAV && checkmap[y][x] == 0)
			{
				//Create a new shape
				if(t_mask_el==NULL){
					t_mask_el = malloc(sizeof(mask_el));
					t_mask_el->shape = malloc((XRES/CELL)*(YRES/CELL));
					memset(t_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					t_mask_el->shapeout = 0;
					t_mask_el->next = NULL;
					c_mask_el = t_mask_el;
				} else {
					c_mask_el->next = malloc(sizeof(mask_el));
					c_mask_el = c_mask_el->next;
					c_mask_el->shape = malloc((XRES/CELL)*(YRES/CELL));
					memset(c_mask_el->shape, 0, (XRES/CELL)*(YRES/CELL));
					c_mask_el->shapeout = 0;
					c_mask_el->next = NULL;
				}
				//Fill the shape
				grav_mask_r(x, y, checkmap, c_mask_el->shape, &c_mask_el->shapeout);
			}
		}
	}
	c_mask_el = t_mask_el;
	memset(gravmask, 0, sizeof(gravmask));
	while(c_mask_el!=NULL)
	{
		char *cshape = c_mask_el->shape;
		for(x = 0; x < XRES/CELL; x++)
		{
			for(y = 0; y < YRES/CELL; y++)
			{
				if(cshape[y*(XRES/CELL)+x]){
					if(c_mask_el->shapeout)
						gravmask[y][x] = 0xFFFFFFFF;
					else
						gravmask[y][x] = 0x00000000;
				}
			}
		}
		c_mask_el = c_mask_el->next;	
	}
	mask_free(t_mask_el);
}
