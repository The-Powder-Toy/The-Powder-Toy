#include <stdint.h>
#include <math.h>
#include <defines.h>
#include <powder.h>
#include <air.h>
#include <misc.h>

int isplayer = 0;
float player[27]; //[0] is a command cell, [3]-[18] are legs positions, [19] is index, [19]-[26] are accelerations

particle *parts;
particle *cb_parts;

unsigned char bmap[YRES/CELL][XRES/CELL];
unsigned char emap[YRES/CELL][XRES/CELL];

unsigned char cb_bmap[YRES/CELL][XRES/CELL];
unsigned char cb_emap[YRES/CELL][XRES/CELL];

int pfree;

unsigned pmap[YRES][XRES];
unsigned cb_pmap[YRES][XRES];

static int pn_junction_sprk(int x, int y, int pt)
{
    unsigned r = pmap[y][x];
    if((r & 0xFF) != pt)
        return 0;
    r >>= 8;
    if(parts[r].type != pt)
        return 0;

    parts[r].ctype = pt;
    parts[r].type = PT_SPRK;
    parts[r].life = 4;
    return 1;
}

static void photoelectric_effect(int nx, int ny)
{
    unsigned r = pmap[ny][nx];

    if((r&0xFF) == PT_PSCN) {
        if((pmap[ny][nx-1] & 0xFF) == PT_NSCN ||
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
static int eval_move(int pt, int nx, int ny, unsigned *rr)
{
    unsigned r;

    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES)
        return 0;

    r = pmap[ny][nx];
    if(r && (r>>8)<NPART)
        r = (r&~0xFF) | parts[r>>8].type;
    if(rr)
        *rr = r;

    if((r&0xFF)==PT_VOID || (r&0xFF)==PT_BHOL)
        return 1;

    if(pt==PT_PHOT&&(
                (r&0xFF)==PT_GLAS || (r&0xFF)==PT_PHOT ||
                (r&0xFF)==PT_CLNE || (r&0xFF)==PT_PCLN ||
                (r&0xFF)==PT_GLOW || (r&0xFF)==PT_WATR ||
                (r&0xFF)==PT_DSTW || (r&0xFF)==PT_SLTW ||
                (r&0xFF)==PT_ISOZ || (r&0xFF)==PT_ISZS ||
		(r&0xFF)==PT_FILT ||
		((r&0xFF)==PT_LCRY&&parts[r>>8].life > 5)))
        return 2;

    if(pt==PT_STKM)  //Stick man's head shouldn't collide
        return 2;
    if((pt==PT_BIZR||pt==PT_BIZRG)&&(r&0xFF)==PT_FILT)
	    return 2;
    if(bmap[ny/CELL][nx/CELL]==WL_ALLOWGAS && ptypes[pt].falldown!=0 && pt!=PT_FIRE && pt!=PT_SMKE)
        return 0;
    if(ptypes[pt].falldown!=2 && bmap[ny/CELL][nx/CELL]==WL_ALLOWLIQUID)
        return 0;
    if((pt==PT_NEUT ||pt==PT_PHOT) && bmap[ny/CELL][nx/CELL]==WL_EWALL && !emap[ny/CELL][nx/CELL])
        return 0;
    if(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[ny/CELL][nx/CELL])
        return 2;
    if(bmap[ny/CELL][nx/CELL]==WL_ALLOWAIR)
        return 0;

    if(ptypes[pt].falldown!=1 && bmap[ny/CELL][nx/CELL]==WL_ALLOWSOLID)
        return 0;
    if(r && (r&0xFF) < PT_NUM){
		if(ptypes[pt].properties&TYPE_ENERGY && ptypes[(r&0xFF)].properties&TYPE_ENERGY)
			return 2;
	
		if(pt==PT_NEUT && ptypes[(r&0xFF)].properties&PROP_NEUTPASS)
			return 2;
		if(pt==PT_NEUT && ptypes[(r&0xFF)].properties&PROP_NEUTPENETRATE)
			return 1;
		if((r&0xFF)==PT_NEUT && ptypes[pt].properties&PROP_NEUTPENETRATE)
			return 0;
    }
	
    if (r && ((r&0xFF) >= PT_NUM || (ptypes[pt].weight <= ptypes[(r&0xFF)].weight)))
        return 0;

	if(pt == PT_PHOT)
        return 2;

    return 1;
}

static void create_cherenkov_photon(int pp);
static void create_gain_photon(int pp);

int try_move(int i, int x, int y, int nx, int ny)
{
    unsigned r, e;

    if(x==nx && y==ny)
        return 1;

    e = eval_move(parts[i].type, nx, ny, &r);
	if((pmap[ny][nx]&0xFF)==PT_INVIS && (pv[ny/CELL][nx/CELL]>4.0f ||pv[ny/CELL][nx/CELL]<-4.0f))
	    return 1;
    /* half-silvered mirror */
    if(!e && parts[i].type==PT_PHOT &&
            (((r&0xFF)==PT_BMTL && rand()<RAND_MAX/2) ||
             (pmap[y][x]&0xFF)==PT_BMTL))
        e = 2;

    if(!e)
    {
        if(!legacy_enable && parts[i].type==PT_PHOT)
        {
            if((r & 0xFF) == PT_COAL || (r & 0xFF) == PT_BCOL)
                parts[r>>8].temp = parts[i].temp;

            if((r & 0xFF) < PT_NUM && ptypes[r&0xFF].hconduct)
                parts[i].temp = parts[r>>8].temp = restrict_flt((parts[r>>8].temp+parts[i].temp)/2, MIN_TEMP, MAX_TEMP);
        }
        return 0;
    }

    if(e == 2)
    {
        if(parts[i].type == PT_PHOT && (r&0xFF)==PT_GLOW && !parts[r>>8].life)
            if(rand() < RAND_MAX/30)
            {
                parts[r>>8].life = 120;
                create_gain_photon(i);
            }
	if(parts[i].type == PT_PHOT && (r&0xFF)==PT_FILT)
            {
		int temp_bin = (int)((parts[r>>8].temp-273.0f)*0.025f);
		if(temp_bin < 0) temp_bin = 0;
		if(temp_bin > 25) temp_bin = 25;
		parts[i].ctype = 0x1F << temp_bin;
	    }
        if(parts[i].type == PT_NEUT && (r&0xFF)==PT_GLAS) {
            if(rand() < RAND_MAX/10)
                create_cherenkov_photon(i);
        }
	if((parts[i].type==PT_BIZR||parts[i].type==PT_BIZRG) && (r&0xFF)==PT_FILT)
	    {
		int temp_bin = (int)((parts[r>>8].temp-273.0f)*0.025f);
		if(temp_bin < 0) temp_bin = 0;
		if(temp_bin > 25) temp_bin = 25;
		parts[i].ctype = 0x1F << temp_bin;
	    }
        return 1;
    }

    if((r&0xFF)==PT_VOID)
    {
        parts[i].type=PT_NONE;
        return 0;
    }
    if((r&0xFF)==PT_BHOL)
    {
        parts[i].type=PT_NONE;
        if(!legacy_enable)
        {
            parts[r>>8].temp = restrict_flt(parts[r>>8].temp+parts[i].temp/2, MIN_TEMP, MAX_TEMP);//3.0f;
        }

        return 0;
    }
    if((pmap[ny][nx]&0xFF)==PT_CNCT)
	return 0;
    if(parts[i].type==PT_CNCT && y<ny && (pmap[y+1][x]&0xFF)==PT_CNCT)
        return 0;

    if(bmap[ny/CELL][nx/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL])
        return 1;
    if((bmap[y/CELL][x/CELL]==WL_EHOLE && !emap[y/CELL][x/CELL]) && (bmap[ny/CELL][nx/CELL]!=WL_EHOLE && !emap[ny/CELL][nx/CELL]))
        return 0;

    if(r && (r>>8)<NPART && ptypes[r&0xFF].falldown!=2 && bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID)
        return 0;

    if(parts[i].type == PT_PHOT)
        return 1;

    e = r >> 8;
    if(r && e<NPART)
    {
        if(parts[e].type == PT_PHOT)
            return 1;

        parts[e].x += x-nx;
        parts[e].y += y-ny;
    }

    pmap[ny][nx] = (i<<8)|parts[i].type;
    pmap[y][x] = r;

    return 1;
}

static unsigned direction_to_map(float dx, float dy)
{
    return (dx >= 0) |
           (((dx + dy) >= 0) << 1) |     /*  567  */
           ((dy >= 0) << 2) |            /*  4+0  */
           (((dy - dx) >= 0) << 3) |     /*  321  */
           ((dx <= 0) << 4) |
           (((dx + dy) <= 0) << 5) |
           ((dy <= 0) << 6) |
           (((dy - dx) <= 0) << 7);
}

static int is_blocking(int t, int x, int y)
{
    if(t & REFRACT) {
        if(x<0 || y<0 || x>=XRES || y>=YRES)
            return 0;
        if((pmap[y][x] & 0xFF) == PT_GLAS)
            return 1;
        return 0;
    }

    return !eval_move(t, x, y, NULL);
}

static int is_boundary(int pt, int x, int y)
{
    if(!is_blocking(pt,x,y))
        return 0;
    if(is_blocking(pt,x,y-1) && is_blocking(pt,x,y+1) && is_blocking(pt,x-1,y) && is_blocking(pt,x+1,y))
        return 0;
    return 1;
}

static int find_next_boundary(int pt, int *x, int *y, int dm, int *em)
{
    static int dx[8] = {1,1,0,-1,-1,-1,0,1};
    static int dy[8] = {0,1,1,1,0,-1,-1,-1};
    static int de[8] = {0x83,0x07,0x0E,0x1C,0x38,0x70,0xE0,0xC1};
    int i, ii, i0;

    if(*x <= 0 || *x >= XRES-1 || *y <= 0 || *y >= YRES-1)
        return 0;

    if(*em != -1) {
        i0 = *em;
        dm &= de[i0];
    } else
        i0 = 0;

    for(ii=0; ii<8; ii++) {
        i = (ii + i0) & 7;
        if((dm & (1 << i)) && is_boundary(pt, *x+dx[i], *y+dy[i])) {
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

    if(!dx && !dy)
        return 0;

    if(!is_boundary(pt, x, y))
        return 0;

    ldm = direction_to_map(-dy, dx);
    rdm = direction_to_map(dy, -dx);
    lx = rx = x;
    ly = ry = y;
    lv = rv = 1;
    lm = rm = -1;

    j = 0;
    for(i=0; i<SURF_RANGE; i++) {
        if(lv)
            lv = find_next_boundary(pt, &lx, &ly, ldm, &lm);
        if(rv)
            rv = find_next_boundary(pt, &rx, &ry, rdm, &rm);
        j += lv + rv;
        if(!lv && !rv)
            break;
    }

    if(j < NORMAL_MIN_EST)
        return 0;

    if((lx == rx) && (ly == ry))
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

    for(i=0; i<NORMAL_INTERP; i++) {
        x = (int)(x0 + 0.5f);
        y = (int)(y0 + 0.5f);
        if(is_boundary(pt, x, y))
            break;
        x0 += dx;
        y0 += dy;
    }
    if(i >= NORMAL_INTERP)
        return 0;

    if(pt == PT_PHOT)
        photoelectric_effect(x, y);

    return get_normal(pt, x, y, dx, dy, nx, ny);
}

void kill_part(int i)
{
    int x, y;

    if(parts[i].type != PT_PHOT) {
        x = (int)(parts[i].x+0.5f);
        y = (int)(parts[i].y+0.5f);

        if(x>=0 && y>=0 && x<XRES && y<YRES)
            pmap[y][x] = 0;
    }

    parts[i].type = PT_NONE;
    parts[i].life = pfree;
    pfree = i;
}

#if defined(WIN32) && !defined(__GNUC__)
_inline int create_part(int p, int x, int y, int t)
#else
inline int create_part(int p, int x, int y, int t)
#endif
{
    int i;

    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return -1;

    if(t==SPC_HEAT||t==SPC_COOL)
    {
        if((pmap[y][x]&0xFF)!=PT_NONE&&(pmap[y][x]&0xFF)<PT_NUM)
        {
            if(t==SPC_HEAT&&parts[pmap[y][x]>>8].temp<MAX_TEMP)
            {
				if((pmap[y][x]&0xFF)==PT_PUMP)
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 0.1f, MIN_TEMP, MAX_TEMP);
                else 
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 4.0f, MIN_TEMP, MAX_TEMP);
            }
            if(t==SPC_COOL&&parts[pmap[y][x]>>8].temp>MIN_TEMP)
            {
				if((pmap[y][x]&0xFF)==PT_PUMP)
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp - 0.1f, MIN_TEMP, MAX_TEMP);
                else 
					parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp - 4.0f, MIN_TEMP, MAX_TEMP);
            }
            return pmap[y][x]>>8;
        }
        else
        {
            return -1;
        }
    }
    if(t==SPC_AIR)
    {
        pv[y/CELL][x/CELL] += 0.03f;
        if(y+CELL<YRES)
            pv[y/CELL+1][x/CELL] += 0.03f;
        if(x+CELL<XRES)
        {
            pv[y/CELL][x/CELL+1] += 0.03f;
            if(y+CELL<YRES)
                pv[y/CELL+1][x/CELL+1] += 0.03f;
        }
        return -1;
    }
    if(t==SPC_VACUUM)
    {
        pv[y/CELL][x/CELL] -= 0.03f;
        if(y+CELL<YRES)
            pv[y/CELL+1][x/CELL] -= 0.03f;
        if(x+CELL<XRES)
        {
            pv[y/CELL][x/CELL+1] -= 0.03f;
            if(y+CELL<YRES)
                pv[y/CELL+1][x/CELL+1] -= 0.03f;
        }
        return -1;
    }

    if(t==PT_SPRK)
    {
        if((pmap[y][x]&0xFF)!=PT_METL &&
                (pmap[y][x]&0xFF)!=PT_PSCN &&
                (pmap[y][x]&0xFF)!=PT_NSCN &&
                (pmap[y][x]&0xFF)!=PT_NTCT &&
                (pmap[y][x]&0xFF)!=PT_PTCT &&
                (pmap[y][x]&0xFF)!=PT_WATR &&
                (pmap[y][x]&0xFF)!=PT_SLTW &&
                (pmap[y][x]&0xFF)!=PT_BMTL &&
                (pmap[y][x]&0xFF)!=PT_RBDM &&
                (pmap[y][x]&0xFF)!=PT_LRBD &&
                (pmap[y][x]&0xFF)!=PT_ETRD &&
                (pmap[y][x]&0xFF)!=PT_BRMT &&
                (pmap[y][x]&0xFF)!=PT_NBLE &&
                (pmap[y][x]&0xFF)!=PT_IRON &&
				(pmap[y][x]&0xFF)!=PT_INST &&
                (pmap[y][x]&0xFF)!=PT_INWR)
            return -1;
	if(parts[pmap[y][x]>>8].life!=0)
		return -1;
        parts[pmap[y][x]>>8].type = PT_SPRK;
        parts[pmap[y][x]>>8].life = 4;
        parts[pmap[y][x]>>8].ctype = pmap[y][x]&0xFF;
        pmap[y][x] = (pmap[y][x]&~0xFF) | PT_SPRK;
        return pmap[y][x]>>8;
    }

    if(p==-1)
    {
        if(pmap[y][x])
            return -1;
        if(pfree == -1)
            return -1;
        i = pfree;
        pfree = parts[i].life;
    }
    else
        i = p;

    if(t==PT_GLAS)
    {
        parts[i].pavg[1] = pv[y/CELL][x/CELL];
    }
    if(t!=PT_STKM)
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
    if(t==PT_ACID)
    {
        parts[i].life = 75;
    }
    /*Testing
    if(t==PT_WOOD){
    	parts[i].life = 150;
    }
    End Testing*/
	if(t==PT_WARP) {
		parts[i].life = rand()%95+70;
	}
    if(t==PT_FUSE) {
        parts[i].life = 50;
        parts[i].tmp = 50;
    }
    if(t==PT_DEUT)
	    parts[i].life = 10;
	if(t==PT_BRAY)
		parts[i].life = 30;
	if(t==PT_PUMP)
		parts[i].life= 10;
    if(t==PT_FSEP)
        parts[i].life = 50;
    if(t==PT_COAL) {
        parts[i].life = 110;
        parts[i].tmp = 50;
    }
    if(t==PT_FRZW)
	    parts[i].life = 100;
    if(t==PT_PIPE)
	parts[i].life = 60;
    if(t==PT_BCOL)
        parts[i].life = 110;
    if(t==PT_FIRE)
        parts[i].life = rand()%50+120;
    if(t==PT_PLSM)
        parts[i].life = rand()%150+50;
    if(t==PT_HFLM)
        parts[i].life = rand()%150+50;
    if(t==PT_LAVA)
        parts[i].life = rand()%120+240;
    if(t==PT_NBLE)
        parts[i].life = 0;
    if(t==PT_ICEI)
        parts[i].ctype = PT_WATR;
    if(t==PT_NEUT)
    {
        float r = (rand()%128+128)/127.0f;
        float a = (rand()%360)*3.14159f/180.0f;
        parts[i].life = rand()%480+480;
        parts[i].vx = r*cosf(a);
        parts[i].vy = r*sinf(a);
    }
    if(t==PT_MORT)
    {
        parts[i].vx = 2;
    }
    if(t==PT_PHOT)
    {
        float a = (rand()%8) * 0.78540f;
        parts[i].life = 680;
        parts[i].ctype = 0x3FFFFFFF;
        parts[i].vx = 3.0f*cosf(a);
        parts[i].vy = 3.0f*sinf(a);
    }

    if(t!=PT_STKM)// && t!=PT_PHOT && t!=PT_NEUT)  is this needed? it breaks floodfill
        pmap[y][x] = t|(i<<8);
    else if(t==PT_STKM)
    {
        if(isplayer==0)
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
    }

    return i;
}

static void create_gain_photon(int pp)
{
    float xx, yy;
    int i, lr, temp_bin, nx, ny;

    if(pfree == -1)
        return;
    i = pfree;

    lr = rand() % 2;

    if(lr) {
        xx = parts[pp].x - 0.3*parts[pp].vy;
        yy = parts[pp].y + 0.3*parts[pp].vx;
    } else {
        xx = parts[pp].x + 0.3*parts[pp].vy;
        yy = parts[pp].y - 0.3*parts[pp].vx;
    }

    nx = (int)(xx + 0.5f);
    ny = (int)(yy + 0.5f);

    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES)
        return;

    if((pmap[ny][nx] & 0xFF) != PT_GLOW)
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

    temp_bin = (int)((parts[i].temp-273.0f)*0.25f);
    if(temp_bin < 0) temp_bin = 0;
    if(temp_bin > 25) temp_bin = 25;
    parts[i].ctype = 0x1F << temp_bin;
}

static void create_cherenkov_photon(int pp)
{
    int i, lr, nx, ny;
    float r, eff_ior;

    if(pfree == -1)
        return;
    i = pfree;

    nx = (int)(parts[pp].x + 0.5f);
    ny = (int)(parts[pp].y + 0.5f);
    if((pmap[ny][nx] & 0xFF) != PT_GLAS)
        return;

    if(hypotf(parts[pp].vx, parts[pp].vy) < 1.44f)
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

    if(lr) {
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

    if(x<0 || y<0 || x>=XRES || y>=YRES)
        return;
    i = pmap[y][x];
    if(!i || (i>>8)>=NPART)
        return;
    if((parts[i>>8].type==SLALT)||SLALT==0)
	{
		kill_part(i>>8);
		pmap[y][x] = 0;	
	}
    else if(ptypes[parts[i>>8].type].menusection==SEC)
	{
		kill_part(i>>8);
		pmap[y][x] = 0;
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

    if(!*wm)
        return -1;

    for(i=0; i<30; i++)
        if(*wm & (1<<i)) {
            if(i < w0)
                w0 = i;
            if(i > wM)
                wM = i;
        }

    if(wM-w0 < 5)
        return (wM+w0)/2;

    i = rand() % (wM-w0-3);
    i += w0;

    *wm &= 0x1F << i;
    return i + 2;
}

void set_emap(int x, int y)
{
    int x1, x2;

    if(!is_wire_off(x, y))
        return;

    // go left as far as possible
    x1 = x2 = x;
    while(x1>0)
    {
        if(!is_wire_off(x1-1, y))
            break;
        x1--;
    }
    while(x2<XRES/CELL-1)
    {
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
            if(is_wire_off(x, y-1))
            {
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
            if(is_wire_off(x, y+1))
            {
                if(x==x1 || x==x2 || y<0 ||
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
	if(t==PT_INSL)//to keep electronics working
	{
		int pmr = pmap[(int)((parts[ci].y + parts[ni].y)/2)][(int)((parts[ci].x + parts[ni].x)/2)];
		if((pmr>>8) < NPART && pmr)
			return parts[pmr>>8].type;
		else 
			return PT_NONE;
	}
	else
	{
		int pmr2 = pmap[(int)((parts[ci].y + parts[ni].y)/2+0.5f)][(int)((parts[ci].x + parts[ni].x)/2+0.5f)];//seems to be more accurate.
		if((pmr2>>8) < NPART && pmr2)
		{
			if(parts[pmr2>>8].type==t)
				return t;
		}
		else
			return PT_NONE;
	}
}


int nearest_part(int ci, int t)
{
    int distance = sqrt(pow(XRES, 2)+pow(YRES, 2));
    int ndistance = 0;
    int id = -1;
    int i = 0;
    int cx = (int)parts[ci].x;
    int cy = (int)parts[ci].y;
    for(i=0; i<NPART; i++)
    {
        if(parts[i].type==t&&!parts[i].life&&i!=ci)
        {
            ndistance = abs((cx-parts[i].x)+(cy-parts[i].y));// Faster but less accurate  Older: sqrt(pow(cx-parts[i].x, 2)+pow(cy-parts[i].y, 2));
            if(ndistance<distance)
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
    int i, j, x, y, t, nx, ny, r, a, s, lt, rt, fe, nt, lpv, nearp, pavg, nnx, nny, q, golnum, goldelete, z, ctype, temp, trade, docontinue, nxx, nyy, nxi, nyi;
    float mv, dx, dy, ix, iy, lx, ly, d, pp, nrx, nry, dp, rr, rrr;
    float nn, ct1, ct2;
    float pt = R_TEMP;
    float c_heat = 0.0f;
    int h_count = 0;
    int starti = (start*-1);
	if(sys_pause&&!framerender)
                return;
    if(ISGRAV==1)
	{
		ISGRAV = 0;
		GRAV ++;
		GRAV_R = 60;
		GRAV_G = 0;
		GRAV_B = 0;
		GRAV_R2 = 30;
		GRAV_G2 = 30;
		GRAV_B2 = 0;
		for( q = 0;q <= GRAV;q++)
		{
			if(GRAV_R >0 && GRAV_G==0)
			{
				GRAV_R--;
				GRAV_B++;
			}
			if(GRAV_B >0 && GRAV_R==0)
			{
				GRAV_B--;
				GRAV_G++;
			}
			if(GRAV_G >0 && GRAV_B==0)
			{
				GRAV_G--;
				GRAV_R++;
			}
			if(GRAV_R2 >0 && GRAV_G2==0)
			{
				GRAV_R2--;
				GRAV_B2++;
			}
			if(GRAV_B2 >0 && GRAV_R2==0)
			{
				GRAV_B2--;
				GRAV_G2++;
			}
			if(GRAV_G2 >0 && GRAV_B2==0)
			{
				GRAV_G2--;
				GRAV_R2++;
			}
		}
		if(GRAV>180) GRAV = 0;
	
	}
	if(ISLOVE==1)
	{
	    ISLOVE = 0;
	    for(ny=0;ny<YRES-4;ny++)
	    {
		for(nx=0;nx<XRES-4;nx++)
		{
			r=pmap[ny][nx];
			if((r>>8)>=NPART || !r)
			{
                               	continue;
			}
			else if((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&parts[r>>8].type==PT_LOVE)
				parts[r>>8].type = PT_NONE;
			else if(parts[r>>8].type==PT_LOVE)
			{
				love[nx/9][ny/9] = 1;
			}
			
		}
	    }
	    for(nx=9;nx<=XRES-18;nx++)
	    {
		for(ny=9;ny<=YRES-7;ny++)
		{
			if(love[nx/9][ny/9]==1)
			{
			    for( nnx=0;nnx<9;nnx++)
				for( nny=0;nny<9;nny++)
				{
					if(ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
					{
						rt=pmap[ny+nny][nx+nnx];
						if((rt>>8)>=NPART)
						{
							continue;
						}
						if(!rt&&loverule[nnx][nny]==1)
							create_part(-1,nx+nnx,ny+nny,PT_LOVE);
						else if(!rt)
							continue;
						else if(parts[rt>>8].type==PT_LOVE&&loverule[nnx][nny]==0)
							parts[rt>>8].type=PT_NONE;
						
					}
				}
			}
			love[nx/9][ny/9]=0;
		}
	    }
	}
	if(ISLOLZ==1)
	{
	    ISLOLZ = 0;
	    for(ny=0;ny<YRES-4;ny++)
	    {
		for(nx=0;nx<XRES-4;nx++)
		{
			r=pmap[ny][nx];
			if((r>>8)>=NPART || !r)
			{
                               	continue;
			}
			else if((ny<9||nx<9||ny>YRES-7||nx>XRES-10)&&parts[r>>8].type==PT_LOLZ)
				parts[r>>8].type = PT_NONE;
			else if(parts[r>>8].type==PT_LOLZ)
			{
				lolz[nx/9][ny/9] = 1;
			}
			
		}
	    }
	    for(nx=9;nx<=XRES-18;nx++)
	    {
		for(ny=9;ny<=YRES-7;ny++)
		{
			if(lolz[nx/9][ny/9]==1)
			{
			    for( nnx=0;nnx<9;nnx++)
				for( nny=0;nny<9;nny++)
				{
					if(ny+nny>0&&ny+nny<YRES&&nx+nnx>=0&&nx+nnx<XRES)
					{
						rt=pmap[ny+nny][nx+nnx];
						if((rt>>8)>=NPART)
						{
							continue;
						}
						if(!rt&&lolzrule[nny][nnx]==1)
							create_part(-1,nx+nnx,ny+nny,PT_LOLZ);
						else if(!rt)
							continue;
						else if(parts[rt>>8].type==PT_LOLZ&&lolzrule[nny][nnx]==0)
							parts[rt>>8].type=PT_NONE;
						
					}
				}
			}
			lolz[nx/9][ny/9]=0;
		}
	    }
	}
	if(ISGOL==1&&CGOL>=GSPEED)//GSPEED is frames per generation
	{
		int createdsomething = 0;
		for(nx=4;nx<XRES-4;nx++)
		for(ny=4;ny<YRES-4;ny++)
		{
			CGOL=0;
			ISGOL=0;
			r = pmap[ny][nx];
			if((r>>8)>=NPART || !r)
			{
				gol[nx][ny] = 0;
                               	continue;
			}
			else 
				for( golnum=1;golnum<NGOL;golnum++)
					if(parts[r>>8].type==golnum+77)
					{
						gol[nx][ny] = golnum;
						for( nnx=-1;nnx<2;nnx++)
						  for( nny=-1;nny<2;nny++)//it will count itself as its own neighbor, which is needed, but will have 1 extra for delete check
						{
							if(ny+nny<4&&nx+nnx<4){//any way to make wrapping code smaller?
							  gol2[XRES-5][YRES-5][golnum] ++;
							  gol2[XRES-5][YRES-5][0] ++;
							}
							else if(ny+nny<4&&nx+nnx>=XRES-4){
							  gol2[4][YRES-5][golnum] ++;
							  gol2[4][YRES-5][0] ++;
							}
							else if(ny+nny>=YRES-4&&nx+nnx<4){
							  gol2[XRES-5][4][golnum] ++;
							  gol2[XRES-5][4][0] ++;
							}
							else if(nx+nnx<4){
							  gol2[XRES-5][ny+nny][golnum] ++;
							  gol2[XRES-5][ny+nny][0] ++;
							}
							else if(ny+nny<4){
							  gol2[nx+nnx][YRES-5][golnum] ++;
							  gol2[nx+nnx][YRES-5][0] ++;
							}
							else if(ny+nny>=YRES-4&&nx+nnx>=XRES-4){
							  gol2[4][4][golnum] ++;
							  gol2[4][4][0] ++;
							}
							else if(ny+nny>=YRES-4){
							  gol2[nx+nnx][4][golnum] ++;
							  gol2[nx+nnx][4][0] ++;
							}
							else if(nx+nnx>=XRES-4){
							  gol2[4][ny+nny][golnum] ++;
							  gol2[4][ny+nny][0] ++;
							}
							else{
							  gol2[nx+nnx][ny+nny][golnum] ++;
							  gol2[nx+nnx][ny+nny][0] ++;
							}
						}
					}
		}
		for(nx=4;nx<XRES-4;nx++)
		  for(ny=4;ny<YRES-4;ny++)
		{
			int neighbors = gol2[nx][ny][0];
			if(neighbors==0)
				continue;
			for( golnum = 1;golnum<NGOL;golnum++)
			  for( goldelete = 0;goldelete<9;goldelete++)
				{
					if(neighbors==goldelete&&gol[nx][ny]==0&&grule[golnum][goldelete]>=2&&gol2[nx][ny][golnum]>=(goldelete%2)+goldelete/2)
					{
						if(create_part(-1,nx,ny,golnum+77))
							createdsomething = 1;
					}
					else if(neighbors-1==goldelete&&gol[nx][ny]==golnum&&(grule[golnum][goldelete]==0||grule[golnum][goldelete]==2))//subtract 1 because it counted itself
						parts[pmap[ny][nx]>>8].type = PT_NONE;
				}
			gol2[nx][ny][0] = 0;
			for( z = 1;z<NGOL;z++)
				gol2[nx][ny][z] = 0;
		}
		if(createdsomething)
			GENERATION ++;
	}
	//if(ISWIRE==1)
	//{
	//	CGOL = 0;
	//	ISWIRE = 0;
	//}
	if(ISWIRE==1)
	{
	 for( q = 0;q<99;q++)
		if(!wireless[q][1])
		{
			wireless[q][0] = 0;
		}
		else
			wireless[q][1] = 0;
	}
    for(i=start; i<(NPART-starti); i+=inc)
        if(parts[i].type)
        {
			lx = parts[i].x;
            ly = parts[i].y;
            t = parts[i].type;
                if (ptypes[t].update_func)
                {
                        if (ptypes[t].update_func (i))
                                goto killed;
                }
            //printf("parts[%d].type: %d\n", i, parts[i].type);


            if(parts[i].life && t!=PT_ACID  && t!=PT_COAL && t!=PT_WOOD && t!=PT_NBLE && t!=PT_SWCH && t!=PT_STKM && t!=PT_FUSE && t!=PT_FSEP && t!=PT_BCOL && t!=PT_GOL && t!=PT_CRAC && t!=PT_DEUT)
            {
                if(!(parts[i].life==10&&(parts[i].type==PT_LCRY||parts[i].type==PT_PCLN||parts[i].type==PT_HSWC||parts[i].type==PT_PUMP)))
                    parts[i].life--;
                if(parts[i].life<=0 && t!=PT_METL && t!=PT_ARAY && t!=PT_IRON && t!=PT_FIRW && t!=PT_PCLN && t!=PT_HSWC && t!=PT_PUMP && t!=PT_WATR && t!=PT_RBDM && t!=PT_LRBD && t!=PT_SLTW && t!=PT_BRMT && t!=PT_PSCN && t!=PT_NSCN && t!=PT_NTCT && t!=PT_PTCT && t!=PT_BMTL && t!=PT_SPRK && t!=PT_LAVA && t!=PT_ETRD&&t!=PT_LCRY && t!=PT_INWR && t!=PT_GLOW && t!= PT_FOG && t!=PT_PIPE && t!=PT_FRZW &&(t!=PT_ICEI&&parts[i].ctype!=PT_FRZW)&&t!=PT_INST && t!=PT_SHLD1&& t!=PT_SHLD2&& t!=PT_SHLD3&& t!=PT_SHLD4)
                {
                    kill_part(i);
                    continue;
                }
                if(parts[i].life<=0 && t==PT_SPRK)
                {
                    t = parts[i].ctype;
                    if(t!=PT_METL&&t!=PT_IRON&&t!=PT_BMTL&&t!=PT_BRMT&&t!=PT_LRBD&&t!=PT_RBDM&&t!=PT_BTRY&&t!=PT_NBLE)
                        parts[i].temp = R_TEMP + 273.15f;
                    if(!t)
                        t = PT_METL;
                    parts[i].type = t;
                    parts[i].life = 4;
                    if(t == PT_WATR)
                        parts[i].life = 64;
                    if(t == PT_SLTW)
                        parts[i].life = 54;
		    if(t == PT_SWCH)
			parts[i].life = 15;
                }
            }

            if(t==PT_SPRK&&parts[i].ctype==PT_SPRK)
            {
                kill_part(i);
                continue;
            }

            x = (int)(parts[i].x+0.5f);
            y = (int)(parts[i].y+0.5f);


            if(x<0 || y<0 || x>=XRES || y>=YRES ||
                    ((bmap[y/CELL][x/CELL]==WL_WALL ||
                      bmap[y/CELL][x/CELL]==WL_WALLELEC ||
                      bmap[y/CELL][x/CELL]==WL_ALLOWAIR ||
                      (bmap[y/CELL][x/CELL]==WL_DESTROYALL) ||
                      (bmap[y/CELL][x/CELL]==WL_ALLOWLIQUID && ptypes[t].falldown!=2) ||
                      (bmap[y/CELL][x/CELL]==WL_ALLOWSOLID && ptypes[t].falldown!=1) ||
                      (bmap[y/CELL][x/CELL]==WL_ALLOWGAS && ptypes[t].falldown!=0 && parts[i].type!=PT_FIRE && parts[i].type!=PT_SMKE && parts[i].type!=PT_HFLM) ||
                      (bmap[y/CELL][x/CELL]==WL_DETECT && (t==PT_METL || t==PT_SPRK)) ||
                      (bmap[y/CELL][x/CELL]==WL_EWALL && !emap[y/CELL][x/CELL])) && (t!=PT_STKM)))
            {
                kill_part(i);
                continue;
            }

            vx[y/CELL][x/CELL] *= ptypes[t].airloss;
            vy[y/CELL][x/CELL] *= ptypes[t].airloss;
	    if(t==PT_ANAR)
	    {
		vx[y/CELL][x/CELL] -= ptypes[t].airdrag*parts[i].vx;
		vy[y/CELL][x/CELL] -= ptypes[t].airdrag*parts[i].vy; 
	    }
	    else
	    {
		vx[y/CELL][x/CELL] += ptypes[t].airdrag*parts[i].vx;
		vy[y/CELL][x/CELL] += ptypes[t].airdrag*parts[i].vy;
	    }
            if(t==PT_GAS||t==PT_NBLE||t==PT_PUMP)
            {
				if(t==PT_PUMP && parts[i].life==10)
				{
					if(parts[i].temp>=256.0+273.15)
						parts[i].temp=256.0+273.15;
					if(parts[i].temp<= -256.0+273.15)
						parts[i].temp = -256.0+273.15;
						
					if(pv[y/CELL][x/CELL]<(parts[i].temp-273.15))
						pv[y/CELL][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL]);
					if(y+CELL<YRES && pv[y/CELL+1][x/CELL]<(parts[i].temp-273.15))
						pv[y/CELL+1][x/CELL] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL]);
					if(x+CELL<XRES)
					{
						pv[y/CELL][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL][x/CELL+1]);
						if(y+CELL<YRES)
							pv[y/CELL+1][x/CELL+1] += 0.1f*((parts[i].temp-273.15)-pv[y/CELL+1][x/CELL+1]);
					}
				}
				else
				{
					if(pv[y/CELL][x/CELL]<3.5f)
						pv[y/CELL][x/CELL] += ptypes[t].hotair*(3.5f-pv[y/CELL][x/CELL]);
					if(y+CELL<YRES && pv[y/CELL+1][x/CELL]<3.5f)
						pv[y/CELL+1][x/CELL] += ptypes[t].hotair*(3.5f-pv[y/CELL+1][x/CELL]);
					if(x+CELL<XRES)
					{
						pv[y/CELL][x/CELL+1] += ptypes[t].hotair*(3.5f-pv[y/CELL][x/CELL+1]);
						if(y+CELL<YRES)
							pv[y/CELL+1][x/CELL+1] += ptypes[t].hotair*(3.5f-pv[y/CELL+1][x/CELL+1]);
					}
				}

            }
            else
            {
                pv[y/CELL][x/CELL] += ptypes[t].hotair;
                if(y+CELL<YRES)
                    pv[y/CELL+1][x/CELL] += ptypes[t].hotair;
                if(x+CELL<XRES)
                {
                    pv[y/CELL][x/CELL+1] += ptypes[t].hotair;
                    if(y+CELL<YRES)
                        pv[y/CELL+1][x/CELL+1] += ptypes[t].hotair;
                }
            }

            if((ptypes[t].explosive&2) && pv[y/CELL][x/CELL]>2.5f)
            {
                parts[i].life = rand()%80+180;
                rt = parts[i].type = PT_FIRE;
                parts[i].temp = ptypes[PT_FIRE].heat + (ptypes[rt].flammable/2);
                pv[y/CELL][x/CELL] += 0.25f * CFDS;
                t = PT_FIRE;
            }

            parts[i].vx *= ptypes[t].loss;
            parts[i].vy *= ptypes[t].loss;

            if((t==PT_GOO) && !parts[i].life)
            {
                if(pv[y/CELL][x/CELL]>1.0f)
                {
                    parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL];
                    parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL];
                    parts[i].life = rand()%80+300;
                }
            }
	    else if((t==PT_BCLN) && !parts[i].life)
            {
                if(pv[y/CELL][x/CELL]>4.0f)
                {
                    parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL];
                    parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL];
                    parts[i].life = rand()%40+80;
                }
            }
            else
            {
		if(t==PT_ANAR)
		{
			parts[i].vx -= ptypes[t].advection*vx[y/CELL][x/CELL];
			parts[i].vy -= ptypes[t].advection*vy[y/CELL][x/CELL] + ptypes[t].gravity;
		}
		else{
                parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL];
                parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL] + ptypes[t].gravity;
		
		}
            }

            if(ptypes[t].diffusion)
            {
                parts[i].vx += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
                parts[i].vy += ptypes[t].diffusion*(rand()/(0.5f*RAND_MAX)-1.0f);
            }

            // interpolator
#if defined(WIN32) && !defined(__GNUC__)
            mv = max(fabsf(parts[i].vx), fabsf(parts[i].vy));
#else
            mv = fmaxf(fabsf(parts[i].vx), fabsf(parts[i].vy));
#endif
            if(mv < ISTP)
            {
                parts[i].x += parts[i].vx;
                parts[i].y += parts[i].vy;
                ix = parts[i].x;
                iy = parts[i].y;
            }
            else
            {
                dx = parts[i].vx*ISTP/mv;
                dy = parts[i].vy*ISTP/mv;
                ix = parts[i].x;
                iy = parts[i].y;
                while(1)
                {
                    mv -= ISTP;
                    if(mv <= 0.0f)
                    {
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
                    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES || pmap[ny][nx] || (bmap[ny/CELL][nx/CELL] && bmap[ny/CELL][nx/CELL]!=WL_STREAM))
                    {
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
                            (!bmap[(y+ny)/CELL][(x+nx)/CELL] || bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_STREAM))
                    {
                        if(!pmap[y+ny][x+nx])
                            a = 1;
                        if((pmap[y+ny][x+nx]&0xFF)!=t)
                            nt = 1;
                    }
            if(legacy_enable)
            {
                if(t==PT_WTRV && pv[y/CELL][x/CELL]>4.0f)
                    t = parts[i].type = PT_DSTW;
                if(t==PT_OIL && pv[y/CELL][x/CELL]<-6.0f)
                    t = parts[i].type = PT_GAS;
                if(t==PT_GAS && pv[y/CELL][x/CELL]>6.0f)
                    t = parts[i].type = PT_OIL;
                if(t==PT_DESL && pv[y/CELL][x/CELL]>12.0f)
                    t = parts[i].type = PT_FIRE;
            }
            if(t==PT_GAS && pv[y/CELL][x/CELL]<-6.0f)
                t = parts[i].type = PT_OIL;
            if(t==PT_DESL && pv[y/CELL][x/CELL]>5.0f)      // Only way I know to make it
                t = parts[i].type = PT_FIRE;                // combust under pressure.
            if(t==PT_GAS && pv[y/CELL][x/CELL]>6.0f)
                t = parts[i].type = PT_OIL;
            if(t==PT_BMTL && pv[y/CELL][x/CELL]>2.5f)
                t = parts[i].type = PT_BRMT;
            if(t==PT_BMTL && pv[y/CELL][x/CELL]>1.0f && parts[i].tmp==1)
                t = parts[i].type = PT_BRMT;
            if(t==PT_BRCK && pv[y/CELL][x/CELL]>8.8f)
                t = parts[i].type = PT_STNE;
	    if(t==PT_PIPE && pv[y/CELL][x/CELL]>10.0f)
                t = parts[i].type = PT_BRMT;
	    if(t==PT_PSTE && pv[y/CELL][x/CELL]>0.5f)
                t = parts[i].type = PT_PSTS;
	    if(t==PT_PSTS && pv[y/CELL][x/CELL]<0.5f)
                t = parts[i].type = PT_PSTE;
	    if(t==PT_SHLD1 && pv[y/CELL][x/CELL]>7.0f)
                t = parts[i].type = PT_NONE;
	    if(t==PT_SHLD2 && pv[y/CELL][x/CELL]>15.0f)
                t = parts[i].type = PT_NONE;
	    if(t==PT_SHLD3 && pv[y/CELL][x/CELL]>25.0f)
                t = parts[i].type = PT_NONE;
	    if(t==PT_SHLD4 && pv[y/CELL][x/CELL]>40.0f)
                t = parts[i].type = PT_NONE;
	     if(t==PT_WIFI && pv[y/CELL][x/CELL]>15.0f)
                t = parts[i].type = PT_BRMT;
            //if(t==PT_GLAS && pv[y/CELL][x/CELL]>4.0f)
            //	t = parts[i].type = PT_BGLA;
            if(t==PT_GLAS)
            {
                parts[i].pavg[0] = parts[i].pavg[1];
                parts[i].pavg[1] = pv[y/CELL][x/CELL];
                if(parts[i].pavg[1]-parts[i].pavg[0] > 0.25f || parts[i].pavg[1]-parts[i].pavg[0] < -0.25f)
                {
                    parts[i].type = PT_BGLA;
                }
            }
            if(t==PT_ICEI && pv[y/CELL][x/CELL]>0.8f)
                t = parts[i].type = PT_SNOW;
            if(t==PT_PLUT && 1>rand()%100 && ((int)(5.0f*pv[y/CELL][x/CELL]))>(rand()%1000))
            {
                t = PT_NEUT;
                create_part(i, x, y, t);
            }
	    if((t==PT_ISOZ||t==PT_ISZS) && 1>rand()%200 && ((int)(-4.0f*(pv[y/CELL][x/CELL])))>(rand()%1000))
            {
                t = PT_PHOT;
		rr = (rand()%228+128)/127.0f;
		rrr = (rand()%360)*3.14159f/180.0f;
		parts[i].life = 680;
		parts[i].ctype = 0x3FFFFFFF;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
                create_part(i, x, y, t);
            }
	    if(t==PT_PSTE)
		if(parts[i].temp>747.0f)
		    t = parts[i].type = PT_BRCK;
            if(t==PT_SPRK&&parts[i].ctype==PT_ETRD&&parts[i].life==1)
            {
                nearp = nearest_part(i, PT_ETRD);
                if(nearp!=-1&&parts_avg(i, nearp, PT_INSL)!=PT_INSL)
                {
                    create_line((int)parts[i].x, (int)parts[i].y, (int)parts[nearp].x, (int)parts[nearp].y, 0, 0, PT_PLSM);
                    t = parts[i].type = PT_ETRD;
                    parts[i].ctype = PT_NONE;
                    parts[i].life = 20;
                    parts[nearp].type = PT_SPRK;
                    parts[nearp].life = 9;
                    parts[nearp].ctype = PT_ETRD;
                }
            }

            if(!legacy_enable)
            {
                int ctemp = pv[y/CELL][x/CELL]*2;
                c_heat = 0.0f;
                h_count = 0;
                if(t==PT_ICEI && !parts[i].ctype)
                    parts[i].ctype = PT_WATR;
                if(ptypes[t].hconduct>(rand()%250)&&!(parts[i].type==PT_HSWC&&parts[i].life!=10))
                {
                    for(nx=-1; nx<2; nx++)
                    {
                        for(ny=-1; ny<2; ny++)
                        {
                            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                if(parts[r>>8].type!=PT_NONE&&parts[i].type!=PT_NONE&&ptypes[parts[r>>8].type].hconduct>0&&!(parts[r>>8].type==PT_HSWC&&parts[r>>8].life!=10)&&!(parts[i].type==PT_PHOT&&parts[r>>8].type==PT_FILT)&&!(parts[i].type==PT_BIZR&&parts[r>>8].type==PT_FILT)&&!(parts[i].type==PT_BIZRG&&parts[r>>8].type==PT_FILT)&&!(parts[r>>8].type==PT_BIZR&&parts[i].type==PT_FILT)&&!(parts[r>>8].type==PT_BIZRG&&parts[i].type==PT_FILT))
                                {
                                    h_count++;
                                    c_heat += parts[r>>8].temp;
                                }
                            }
                        }
                    }
                    pt = parts[i].temp = (c_heat+parts[i].temp)/(h_count+1);
                    for(nx=-1; nx<2; nx++)
                    {
                        for(ny=-1; ny<2; ny++)
                        {
                            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                if(parts[r>>8].type!=PT_NONE&&parts[i].type!=PT_NONE&&ptypes[parts[r>>8].type].hconduct>0&&!(parts[r>>8].type==PT_HSWC&&parts[r>>8].life!=10)&&!(parts[i].type==PT_PHOT&&parts[r>>8].type==PT_FILT)&&!(parts[i].type==PT_BIZR&&parts[r>>8].type==PT_FILT)&&!(parts[i].type==PT_BIZRG&&parts[r>>8].type==PT_FILT)&&!(parts[r>>8].type==PT_BIZR&&parts[i].type==PT_FILT)&&!(parts[r>>8].type==PT_BIZRG&&parts[i].type==PT_FILT))
                                {
                                    parts[r>>8].temp = parts[i].temp;
                                }
                            }
                        }
                    }
                    if(pt>=pstates[t].btemp&&pstates[t].burn)
                    {
                        t = parts[i].type = pstates[t].burn;
                        if(t==PT_FIRE||t==PT_PLSM)
                            parts[i].life = rand()%50+120;
                    }
                    else if((pt<=pstates[t].stemp||(t==PT_LAVA&&(pt<=pstates[parts[i].ctype].ltemp)))&&pstates[t].solid)
                    {
                        if(t==PT_LAVA&&parts[i].ctype)
                        {
                            parts[i].life = 0;
                            if(parts[i].ctype==PT_THRM)
                            {
                                parts[i].tmp = 0;
                                parts[i].ctype = PT_BMTL;
                            }
                            if(parts[i].ctype==PT_PLUT)
                            {
                                parts[i].tmp = 0;
                                parts[i].ctype = PT_LAVA;
                            }
                            t = parts[i].type = parts[i].ctype;
                            parts[i].ctype = PT_NONE;
                        }
                        else if(pstates[t].solid==PT_ICEI&&pt<=pstates[t].stemp)
                        {
                            parts[i].ctype = parts[i].type;
                            t = parts[i].type = PT_ICEI;
                        }
                        else
                        {
                            parts[i].life = 0;
                            t = parts[i].type = pstates[t].solid;
                        }
                    }
                    else if((pt>=pstates[t].ltemp&&(pt<=pstates[t].gtemp||!pstates[t].gas)&&pstates[t].state==ST_SOLID&&pstates[t].liquid)||(t==PT_ICEI&&pt>pstates[parts[i].ctype].stemp))
                    {
                        if(pstates[t].liquid==PT_LAVA)
                        {
                            parts[i].life = rand()%120+240;
                            parts[i].ctype = (parts[i].type==PT_BRMT)?PT_BMTL:parts[i].type;
                            parts[i].ctype = (parts[i].ctype==PT_SAND)?PT_GLAS:parts[i].ctype;
                            parts[i].ctype = (parts[i].ctype==PT_BGLA)?PT_GLAS:parts[i].ctype;
                            t = parts[i].type = pstates[t].liquid;
                        }
                        else if(t==PT_ICEI&&parts[i].ctype)
                        {
                            t = parts[i].type = parts[i].ctype;
                            parts[i].ctype = PT_NONE;
                        }
                        else
                        {
                            t = parts[i].type = pstates[t].liquid;
                        }
                    }
                    else if(pt-ctemp<=pstates[t].ltemp&&pstates[t].liquid&&pstates[t].state==ST_GAS)
                    {
                        t = parts[i].type = pstates[t].liquid;
                    }
                    else if(pt-ctemp>=pstates[t].gtemp&&(pstates[t].gas||parts[i].type==PT_LNTG)&&(pstates[t].state==ST_LIQUID||pstates[t].state==ST_SOLID))
                    {
                        if(t==PT_SLTW&&1>rand()%6)
                        {
                            t = parts[i].type = PT_SALT;
                        }
                        else
                        {
                            t = parts[i].type = pstates[t].gas;
			    if(t!=PT_BIZRS)
				pv[y/CELL][x/CELL] += 0.50f;
                            if(t==PT_FIRE)
                                parts[i].life = rand()%50+120;
                            if(t==PT_HFLM)
                                parts[i].life = rand()%50+120;
                        }
                    }
                    if(t==PT_URAN && pv[y/CELL][x/CELL]>0.0f)
                    {
                        float atemp =  parts[i].temp + (-MIN_TEMP);
                        pt = parts[i].temp = (atemp*(1+(pv[y/CELL][x/CELL]/2000)))+MIN_TEMP;
                    }
                    if(t==PT_LAVA)
                    {
                        parts[i].life = restrict_flt((pt-700)/7, 0.0f, 400.0f);
                        if(parts[i].ctype==PT_THRM&&parts[i].tmp>0)
                        {
                            parts[i].tmp--;
                            parts[i].temp = 3500;
                        }
                        if(parts[i].ctype==PT_PLUT&&parts[i].tmp>0)
                        {
                            parts[i].tmp--;
                            parts[i].temp = MAX_TEMP;
                        }
                    }
                    pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
                }
            }
            if(t==PT_PTCT&&parts[i].temp>295.0f)
            {
                pt = parts[i].temp -= 2.5f;
            }
            if(t==PT_NTCT&&parts[i].temp>295.0f)
            {
                pt = parts[i].temp -= 2.5f;
            }

            if(t==PT_WATR || t==PT_ETRD || t==PT_SLTW || t==PT_METL || t==PT_IRON || t==PT_RBDM || t==PT_LRBD || t==PT_BRMT || t==PT_PSCN || t==PT_NSCN || t==PT_NTCT || t==PT_PTCT || t==PT_BMTL || t==PT_SPRK|| t == PT_NBLE || t==PT_INWR)
            {
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
                if(nx>=0 && ny>=0 && nx<XRES/CELL && ny<YRES/CELL)
                {
                    if(t==PT_WATR || t==PT_ETRD || t==PT_SLTW || t==PT_METL || t==PT_IRON || t==PT_RBDM || t==PT_LRBD || t==PT_NSCN || t==PT_NTCT || t==PT_PTCT || t==PT_PSCN || t==PT_BRMT || t==PT_BMTL||t==PT_NBLE || t==PT_INWR)
                    {
                        if(emap[ny][nx]==12 && !parts[i].life)
                        {
                            parts[i].type = PT_SPRK;
                            parts[i].life = 4;
                            parts[i].ctype = t;
                            t = PT_SPRK;
                        }
                    }
                    else if(bmap[ny][nx]==WL_DETECT || bmap[ny][nx]==WL_EWALL || bmap[ny][nx]==WL_ALLOWLIQUID || bmap[ny][nx]==WL_WALLELEC || bmap[ny][nx]==WL_ALLOWALLELEC || bmap[ny][nx]==WL_EHOLE)
                        set_emap(nx, ny);
                }
            }

            nx = x/CELL;
            ny = y/CELL;
            if(bmap[ny][nx]==WL_DETECT && emap[ny][nx]<8)
                set_emap(nx, ny);

            fe = 0;
            if(t==PT_THDR)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_METL || (r&0xFF)==PT_IRON || (r&0xFF)==PT_ETRD || (r&0xFF)==PT_PSCN || (r&0xFF)==PT_NSCN || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_PTCT || (r&0xFF)==PT_BMTL || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD || (r&0xFF)==PT_BRMT || (r&0xFF)==PT_NBLE || (r&0xFF)==PT_INWR) && parts[r>>8].ctype!=PT_SPRK)
                            {
                                t = parts[i].type = PT_NONE;
                                parts[r>>8].ctype = parts[r>>8].type;
                                parts[r>>8].type = PT_SPRK;
                                parts[r>>8].life = 4;
                            }
                            else if((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_SPRK&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT&&(r&0xFF))
                            {
                                pv[y/CELL][x/CELL] += 100.0f;
                                if(legacy_enable&&1>(rand()%200))
                                {
                                    parts[i].life = rand()%50+120;
                                    t = parts[i].type = PT_FIRE;
                                }
                                else
                                {
                                    t = parts[i].type = PT_NONE;
                                }
                            }
                        }
            }
            else if(t==PT_ICEI || t==PT_SNOW)
            {
		    if(parts[i].ctype==PT_FRZW)
		    {
			parts[i].temp -= 1.0f;
			if(parts[i].temp<0)
				parts[i].temp = 0;
					
		    }
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_SALT || (r&0xFF)==PT_SLTW) && 1>(rand()%1000))
                            {
                                t = parts[i].type = PT_SLTW;
                                parts[r>>8].type = PT_SLTW;
                            }
                            if(legacy_enable)
                            {
                                if(((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
                                {
                                    t = parts[i].type = PT_ICEI;
                                    parts[r>>8].type = PT_ICEI;
                                }
                                if(t==PT_SNOW && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 15>(rand()%1000))
                                    t = parts[i].type = PT_WATR;
                            }
                        }
            }
            else if(t==PT_BMTL) {
                if(parts[i].tmp>1) {
                    parts[i].tmp--;
                    for(nx=-1; nx<2; nx++)
                        for(ny=-1; ny<2; ny++)
                            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                rt =parts[r>>8].type;
                                if((rt==PT_METL || rt==PT_IRON) && 1>(rand()/(RAND_MAX/100)))
                                {
                                    parts[r>>8].type=PT_BMTL;
                                    parts[r>>8].tmp=(parts[i].tmp<=7)?parts[i].tmp=1:parts[i].tmp-(rand()%5);//rand()/(RAND_MAX/300)+100;
                                }
                            }
                } else if(parts[i].tmp==1 && 1>rand()%1000) {
                    parts[i].tmp = 0;
                    t = parts[i].type = PT_BRMT;
                }
            }

            else if(t==PT_IRON) {
                for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((((r&0xFF) == PT_SALT && 15>(rand()/(RAND_MAX/700))) ||
                                    ((r&0xFF) == PT_SLTW && 30>(rand()/(RAND_MAX/2000))) ||
                                    ((r&0xFF) == PT_WATR && 5 >(rand()/(RAND_MAX/6000))) ||
                                    ((r&0xFF) == PT_O2   && 2 >(rand()/(RAND_MAX/500))) ||
                                    ((r&0xFF) == PT_LO2))&&
                                    (!(parts[i].life))
                              )
                            {
                                parts[i].type=PT_BMTL;
                                parts[i].tmp=(rand()/(RAND_MAX/10))+20;
                            }
                        }
            }
            else if((t==PT_SPRK||parts[i].type==PT_SPRK) && parts[i].ctype==PT_IRON) {
                for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF) == PT_DSTW && 30>(rand()/(RAND_MAX/1000))) ||
                                    ((r&0xFF) == PT_SLTW && 30>(rand()/(RAND_MAX/1000))) ||
                                    ((r&0xFF) == PT_WATR && 30>(rand()/(RAND_MAX/1000))))
                            {
                                parts[r>>8].type=PT_O2;
                                //parts[r>>8].tmp=(rand()/(RAND_MAX/10))+20;
                            }
                        }
            }
            else if(t==PT_COAL)
            {
                if(parts[i].life<=0) {
                    t = PT_NONE;
                    kill_part(i);
                    create_part(-1, x, y, PT_FIRE);
                    goto killed;
                } else if(parts[i].life < 100) {
                    parts[i].life--;
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
                }
                if((pv[y/CELL][x/CELL] > 4.3f)&&parts[i].tmp>40)
                    parts[i].tmp=39;
                else if(parts[i].tmp<40&&parts[i].tmp>0)
                    parts[i].tmp--;
                else if(parts[i].tmp<=0) {
                    t = PT_NONE;
                    kill_part(i);
                    r = create_part(-1, x, y, PT_BCOL);
                    goto killed;
                }
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
                            {
                                if(parts[i].life>100) {
                                    parts[i].life = 99;
                                }
                            }
                        }
            }
            else if(t==PT_BCOL)
            {
                if(parts[i].life<=0) {
                    t = PT_NONE;
                    kill_part(i);
                    create_part(-1, x, y, PT_FIRE);
                    goto killed;
                } else if(parts[i].life < 100) {
                    parts[i].life--;
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_FIRE);
                }

                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM) && 1>(rand()%500))
                            {
                                if(parts[i].life>100) {
                                    parts[i].life = 99;
                                }
                            }
                        }
            }
			else if(t==PT_ARAY && parts[i].life==0){
				for(nx=-1; nx<2; nx++){
                    for(ny=-1; ny<2; ny++){
						if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny)){
							r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
							if((r&0xFF)==PT_SPRK){
								int destroy = (parts[r>>8].ctype==PT_PSCN)?1:0;
								for (docontinue = 1, nxx = 0, nyy = 0, nxi = nx*-1, nyi = ny*-1; docontinue; nyy+=nyi, nxx+=nxi) {
									if(!(x+nxi+nxx<XRES && y+nyi+nyy<YRES && x+nxi+nxx >= 0 && y+nyi+nyy >= 0)){
										break;
									}
									r = pmap[y+nyi+nyy][x+nxi+nxx];
									if(!((r>>8)>=NPART)) {
										if(!r){
											int nr = create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_BRAY);
											if(nr!=-1&&destroy){
												parts[nr].tmp = 2;
												parts[nr].life = 2;
											}
										} else if(!destroy) {
											if(parts[r>>8].type==PT_BRAY&&parts[r>>8].tmp==0){
												if(nyy!=0 || nxx!=0){
													parts[r>>8].type = PT_BRAY;
													parts[r>>8].life = 1020;
													parts[r>>8].tmp = 1;
												}
												docontinue = 0;
											} else if(parts[r>>8].type==PT_BRAY&&parts[r>>8].tmp==1){
												parts[r>>8].life = 1020;
												//docontinue = 1;
											} else {
												if(nyy!=0 || nxx!=0){
													create_part(-1, x+nxi+nxx, y+nyi+nyy, PT_SPRK);
												}
												docontinue = 0;
											}
										} else if(destroy) {
											if(parts[r>>8].type==PT_BRAY){
												parts[r>>8].life = 1;
												docontinue = 1;
											} else {
												docontinue = 0;
											}
										}
									}
								}
							}
							//parts[i].life = 4;
						}
					}
				}
			}
            else if(t==PT_FUSE)
            {
                if(parts[i].life<=0) {
                    //t = parts[i].life = PT_NONE;
                    kill_part(i);
                    r = create_part(-1, x, y, PT_PLSM);
                    if(r!=-1)
                        parts[r].life = 50;
                    goto killed;
                } else if (parts[i].life < 40) {
                    parts[i].life--;
                    if((rand()%100)==0) {
                        r = create_part(-1, (nx=x+rand()%3-1), (ny=y+rand()%3-1), PT_PLSM);
                        if(r!=-1)
                            parts[r].life = 50;
                    }
                }
                if((pv[y/CELL][x/CELL] > 2.7f)&&parts[i].tmp>40)
                    parts[i].tmp=39;
                else if(parts[i].tmp<40&&parts[i].tmp>0)
                    parts[i].tmp--;
                else if(parts[i].tmp<=0) {
                    //t = PT_NONE;
                    kill_part(i);
                    r = create_part(-1, x, y, PT_FSEP);
                    goto killed;
                }
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_SPRK || ((parts[i].temp>=(273.15+700.0f)) && 1>(rand()%20)))
                            {
                                if(parts[i].life>40) {
                                    parts[i].life = 39;
                                }
                            }
                        }
            }
            else if(t==PT_FSEP)
            {
                if(parts[i].life<=0) {
                    //t = PT_NONE;
                    kill_part(i);
                    r = create_part(-1, x, y, PT_PLSM);
                    if(r!=-1)
                        parts[r].life = 50;
                    goto killed;
                } else if (parts[i].life < 40) {
                    parts[i].life--;
                    if((rand()%10)==0) {
                        r = create_part(-1, (nx=x+rand()%3-1), (ny=y+rand()%3-1), PT_PLSM);
                        if(r!=-1)
                            parts[r].life = 50;
                    }
                }
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_SPRK || (parts[i].temp>=(273.15+400.0f))) && 1>(rand()%15))
                            {
                                if(parts[i].life>40) {
                                    parts[i].life = 39;
                                }
                            }
                        }
            }
            else if(t==PT_NTCT||t==PT_PTCT||t==PT_INWR)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_METL && parts_avg(i, r>>8,PT_INSL)!=PT_INSL)
                            {
                                parts[i].temp = 473.0f;
                            }
                        }
            }
            else if(t==PT_PLNT)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_WATR && 1>(rand()%250))
                            {
                                t = parts[i].type = PT_PLNT;
                                parts[r>>8].type = PT_PLNT;
								parts[r>>8].life = 0;
                            }
                            else if((r&0xFF)==PT_LAVA && 1>(rand()%250))
                            {
                                parts[i].life = 4;
                                t = parts[i].type = PT_FIRE;
                            }
			    else if((r&0xFF)==PT_SMKE && (1>rand()%250))
			    {
				    parts[r>>8].type = PT_NONE;
				    parts[i].life = rand()%60 + 60;
			    }
			    else if((r&0xFF)==PT_WOOD && (1>rand()%20) && abs(nx+ny)<=2 && VINE_MODE)
			    {
				    int nnx = rand()%3 -1;
				    int nny = rand()%3 -1;
				    if(x+nx+nnx>=0 && y+ny+nny>0 &&
					x+nx+nnx<XRES && y+ny+nny<YRES && (nnx || nny))
					{
						if((pmap[y+ny+nny][x+nx+nnx]>>8)>=NPART||pmap[y+ny+nny][x+nx+nnx])
							continue;
						if(create_part(-1,x+nx+nnx,y+ny+nny,PT_VINE))
							parts[pmap[y+ny+nny][x+nx+nnx]>>8].temp = parts[i].temp;
					}
			    }
                            //if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
                            //t = parts[i].type = PT_WATR;
                        }
		if(parts[i].life==2)
		{
		    for(nx=-1; nx<2; nx++)
			for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
				r = pmap[y+ny][x+nx];
				if((r>>8)>=NPART)
                                continue;
				if(!r)
					create_part(-1,x+nx,y+ny,PT_O2);
			}
		    parts[i].life = 0;
		}
            }
	    else if(t==PT_VINE)
	    {
		    nx=(rand()%3)-1;
		    ny=(rand()%3)-1;
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
				r = pmap[y+ny][x+nx];
				if((r>>8)>=NPART)
                                continue;
				if(1>rand()%15)
					parts[i].type=PT_PLNT;
				else if(!r)
				{
					create_part(-1,x+nx,y+ny,PT_VINE);
					parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;
					parts[i].type=PT_PLNT;
				}
			}
	    }
            else if(t==PT_THRM)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_FIRE || (r&0xFF)==PT_PLSM || (r&0xFF)==PT_LAVA))
                            {
                                if(1>(rand()%500)) {
                                    t = parts[i].type = PT_LAVA;
                                    parts[i].ctype = PT_BMTL;
                                    pt = parts[i].temp = 3500.0f;
                                    pv[y/CELL][x/CELL] += 50.0f;
                                } else {
                                    t = parts[i].type = PT_LAVA;
                                    parts[i].life = 400;
                                    parts[i].ctype = PT_THRM;
                                    pt = parts[i].temp = 3500.0f;
                                    parts[i].tmp = 20;
                                }
                            }
                            //if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
                            //t = parts[i].type = PT_WATR;
                        }
            }
            else if(t==PT_WATR||t==PT_DSTW)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10) && legacy_enable)
                            {
                                t = parts[i].type = PT_WTRV;
                            }
                            else if((r&0xFF)==PT_SALT && 1>(rand()%250))
                            {
                                t = parts[i].type = PT_SLTW;
                                parts[r>>8].type = PT_SLTW;
                            }
                            if((((r&0xFF)==PT_WATR||(r&0xFF)==PT_SLTW)&&t==PT_DSTW) && 1>(rand()%500))
                            {
                                t = parts[i].type = PT_WATR;
                            }
                            if(((r&0xFF)==PT_SLTW&&t==PT_DSTW) && 1>(rand()%500))
                            {
                                t = parts[i].type = PT_SLTW;
                            }
                            if(((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && (legacy_enable||pt>12.0f) && 1>(rand()%500))
                            {
                                parts[i].life = 4;
                                t = parts[i].type = PT_FIRE;
                            }
			    if(((r&0xFF)==PT_CNCT&&t==PT_WATR) && 1>(rand()%500))
                            {
                                t = parts[i].type = PT_PSTE;
				parts[r>>8].type = PT_NONE;
                            }
                        }
            }
            else if(t==PT_SLTW)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10) && legacy_enable)
                            {
                                t = parts[i].type = PT_SALT;
                                parts[r>>8].type = PT_WTRV;
                            }
                            else if((r&0xFF)==PT_SALT && 1>(rand()%10000))
                            {
                                parts[r>>8].type = PT_NONE;
                            }
			    if((r&0xFF)==PT_PLNT&&5>(rand()%1000))
				parts[r>>8].type = PT_NONE;
                            if(((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && pt>12.0f && 1>(rand()%500))
                            {
                                parts[i].life = 4;
                                t = parts[i].type = PT_FIRE;

                            }
                        }
            }
            else if(t==PT_WTRV)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(((r&0xFF)==PT_WATR||(r&0xFF)==PT_DSTW||(r&0xFF)==PT_SLTW) && 1>(rand()%1000) && legacy_enable)
                            {
                                t = parts[i].type = PT_WATR;
                                parts[r>>8].type = PT_WATR;
                            }

                            if(((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && pt>12.0f && 1>(rand()%500))
                            {
                                parts[i].life = 4;
                                t = parts[i].type = PT_FIRE;

                            }
                            if(((r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) && 1>(rand()%1000) && legacy_enable)
                            {
                                t = parts[i].type = PT_WATR;
                                if(1>(rand()%1000))
                                    parts[r>>8].type = PT_WATR;
                            }
                        }
            }
            else if(t==PT_YEST)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_DYST && 1>(rand()%30) && !legacy_enable)
                            {
                                t = parts[i].type = PT_DYST;
                            }
                        }
            }
            else if(t==PT_ACID)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)!=PT_ACID)
                            {
                                if ((r&0xFF)==PT_PLEX || (r&0xFF)==PT_NITR || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD)
                                {
                                    t = parts[i].type = PT_FIRE;
                                    parts[i].life = 4;
                                    parts[r>>8].type = PT_FIRE;
                                    parts[r>>8].life = 4;
                                }
                                else if(((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && ptypes[parts[r>>8].type].hardness>(rand()%1000))&&parts[i].life>=50)
                                {
					if(parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)
					{
					parts[i].life--;
					parts[r>>8].type = PT_NONE;
					}
                                }
                                else if (parts[i].life==50)
                                {
                                    parts[i].life = 0;
                                    t = parts[i].type = PT_NONE;
                                }
                            }
                        }
            }
            else if(t==PT_NEUT)
            {
                rt = 3 + (int)pv[y/CELL][x/CELL];
                for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW)
                            {
                                parts[i].vx *= 0.995;
                                parts[i].vy *= 0.995;
                            }
                            if((r&0xFF)==PT_PLUT && rt>(rand()%1000))
                            {
                                if(33>rand()%100)
                                {
                                    create_part(r>>8, x+nx, y+ny, rand()%3 ? PT_LAVA : PT_URAN);
				    parts[r>>8].temp = MAX_TEMP;
				    if(parts[r>>8].type==PT_LAVA){
				    	parts[r>>8].tmp = 100;
					parts[r>>8].ctype = PT_PLUT;
				    }
                                }
                                else
                                {
                                    create_part(r>>8, x+nx, y+ny, PT_NEUT);
                                    parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
                                    parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
                                }
                                pv[y/CELL][x/CELL] += 10.0f * CFDS; //Used to be 2, some people said nukes weren't powerful enough
                                fe ++;
                            }
			    if((r&0xFF)==PT_DEUT && (rt+1)>(rand()%1000))
			    {
				    
				    create_part(r>>8, x+nx, y+ny, PT_NEUT);
                                    parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
                                    parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
				    if(parts[r>>8].life>0)
				    {
					    parts[r>>8].life --;
					    parts[r>>8].temp += (parts[r>>8].life*17);
					    pv[y/CELL][x/CELL] += 6.0f * CFDS;
					    
				    }
				    else 
					    parts[r>>8].type = PT_NONE;
			    }
                            if((r&0xFF)==PT_GUNP && 15>(rand()%1000))
                                parts[r>>8].type = PT_DUST;
                            if((r&0xFF)==PT_DYST && 15>(rand()%1000))
                                parts[r>>8].type = PT_YEST;
                            if((r&0xFF)==PT_YEST) {
                                if(15>(rand()%100000)&&isplayer==0)
                                    parts[r>>8].type = PT_STKM;
                                else
                                    parts[r>>8].type = PT_DYST;
                            }

                            if((r&0xFF)==PT_WATR && 15>(rand()%100))
                                parts[r>>8].type = PT_DSTW;
                            if((r&0xFF)==PT_PLEX && 15>(rand()%1000))
                                parts[r>>8].type = PT_GOO;
                            if((r&0xFF)==PT_NITR && 15>(rand()%1000))
                                parts[r>>8].type = PT_DESL;
                            if((r&0xFF)==PT_PLNT && 5>(rand()%100))
                                parts[r>>8].type = PT_WOOD;
                            if((r&0xFF)==PT_DESL && 15>(rand()%1000))
                                parts[r>>8].type = PT_GAS;
                            if((r&0xFF)==PT_COAL && 5>(rand()%100))
                                parts[r>>8].type = PT_WOOD;
			    if((r&0xFF)==PT_DUST && 5>(rand()%100))
                                parts[r>>8].type = PT_FWRK;
			    if((r&0xFF)==PT_FWRK && 5>(rand()%100))
                                parts[r>>8].ctype = PT_DUST;
                            /*if(parts[r>>8].type>1 && parts[r>>8].type!=PT_NEUT && parts[r>>8].type-1!=PT_NEUT && parts[r>>8].type-1!=PT_STKM &&
                              (ptypes[parts[r>>8].type-1].menusection==SC_LIQUID||
                              ptypes[parts[r>>8].type-1].menusection==SC_EXPLOSIVE||
                              ptypes[parts[r>>8].type-1].menusection==SC_GAS||
                              ptypes[parts[r>>8].type-1].menusection==SC_POWDERS) && 15>(rand()%1000))
                              parts[r>>8].type--;*/
                        }
            }
            else if(t==PT_PHOT)
            {
                rt = 3 + (int)pv[y/CELL][x/CELL];
                for(nx=0; nx<1; nx++)
                    for(ny=0; ny<1; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW)
                            {
                                parts[i].vx *= 0.995;
                                parts[i].vy *= 0.995;
                            }
                        }
		for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    if((r&0xFF)==PT_ISOZ && 5>(rand()%2000))
			    {
				parts[i].vx *= 0.90;
                                parts[i].vy *= 0.90;
                                parts[r>>8].type = PT_PHOT;
				rrr = (rand()%360)*3.14159f/180.0f;
				rr = (rand()%128+128)/127.0f;
				parts[r>>8].life = 680;
				parts[r>>8].ctype = 0x3FFFFFFF;
				parts[r>>8].vx = rr*cosf(rrr);
				parts[r>>8].vy = rr*sinf(rrr);
				pv[y/CELL][x/CELL] -= 15.0f * CFDS;
			    }
			    if((r&0xFF)==PT_ISZS && 5>(rand()%2000))
			    {
				parts[i].vx *= 0.90;
                                parts[i].vy *= 0.90;
                                parts[r>>8].type = PT_PHOT;
				rr = (rand()%228+128)/127.0f;
				rrr = (rand()%360)*3.14159f/180.0f;
				parts[r>>8].life = 680;
				parts[r>>8].ctype = 0x3FFFFFFF;
				parts[r>>8].vx = rr*cosf(rrr);
				parts[r>>8].vy = rr*sinf(rrr);
				pv[y/CELL][x/CELL] -= 15.0f * CFDS;
			    }
			}
            }
            else if(t==PT_MORT) {
                create_part(-1, x, y-1, PT_SMKE);
            }
	    else if(t>77&&t<77+NGOL)
	    {
		if(parts[i].temp>0)
			parts[i].temp -= 50.0f;
		ISGOL=1;
	    }
	    else if(t==PT_LOVE)
		ISLOVE=1;
	    else if(t==PT_LOLZ)
		ISLOLZ=1;
	    else if(t==PT_GRAV)
		ISGRAV=1;
	    else if(t==PT_CRAC)
	    {
			if(pv[y/CELL][x/CELL]<=3&&pv[y/CELL][x/CELL]>=-3)
			{
       		  for(nx=-1; nx<2; nx++)
        	    for(ny=-1; ny<2; ny++)
           	     if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
           	     {
            	        r = pmap[y+ny][x+nx];
						if((r>>8)>=NPART || !r || parts[i].temp>374.0f)
              		      	continue;
              	        if(parts[r>>8].type==PT_WATR&&33>=rand()/(RAND_MAX/100)+1)
                  	{
                        	parts[i].life++;
                        	parts[r>>8].type=PT_NONE;
                    	}
                     }
    	    	}
    	    	else
            	for(nx=-1; nx<2; nx++)
            	    for(ny=-1; ny<2; ny++)
                 	if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                 	{
                    		r = pmap[y+ny][x+nx];
		   	 	if((r>>8)>=NPART)
                    			continue;
			 	if((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_EWALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||
						bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWAIR||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWGAS))
					continue;
                    	 	if((!r)&&parts[i].life>=1)//if nothing then create water
                    		{
                        		create_part(-1,x+nx,y+ny,PT_WATR);
                        		parts[i].life--;
                    	 	}
			}
    	    	for( trade = 0; trade<9;trade ++)
    	    	{
  	  		nx = rand()%5-2;
  	  		ny = rand()%5-2;
  	  		if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
 	   		{
  	      			r = pmap[y+ny][x+nx];
				if((r>>8)>=NPART || !r)
 	             			continue;
   	        		if(parts[r>>8].type==t&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
   	        		{
		   			int temp = parts[i].life - parts[r>>8].life;
					if(temp ==1)
					{
            					parts[r>>8].life ++;
     		       				parts[i].life --;
						trade = 9;
					}
					else if(temp>0)
					{
      		      				parts[r>>8].life += temp/2;
       		     				parts[i].life -= temp/2;
						trade = 9;
					}
    	    			}
   	    		}
    		}
			for(nx=-1; nx<2; nx++)
            	    for(ny=-1; ny<2; ny++)
                	if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                	{
                    		r = pmap[y+ny][x+nx];
		    		if((r>>8)>=NPART || !r)
                    			continue;
		    		if(parts[r>>8].type==PT_FIRE&&parts[i].life>0)
		    		{
					if(parts[i].life<=2)
						parts[i].life --;
					parts[i].life -= parts[i].life/3;
		    		}
			}
			if(parts[i].temp>=374)
				for(nx=-1; nx<2; nx++)
            		for(ny=-1; ny<2; ny++)
                	if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                	{
                   		r = pmap[y+ny][x+nx];
		    		if((r>>8)>=NPART)
                    			continue;
		    		if((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWLIQUID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID))
					continue;
                    		if((!r)&&parts[i].life>=1)//if nothing then create steam
                    		{
                        		create_part(-1,x+nx,y+ny,PT_WTRV);
                        		parts[i].life--;
					parts[i].temp -= 40.0f;
                    		}
                	}
	    }
	    else if(t==PT_FOG)
	    {
		if(parts[i].temp>=373.15)
			t = parts[i].type = PT_WTRV;
		for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    if(pstates[parts[r>>8].type].state==ST_SOLID&&5>=rand()%50&&parts[i].life==0&&!(parts[r>>8].type==PT_CLNE||parts[r>>8].type==PT_PCLN))
			    {
				t = parts[i].type = PT_RIME;
			    }
			    if(parts[r>>8].type==PT_SPRK)
                            {
				parts[i].life += rand()%20;
			    }
			}
	    }
	    else if(t==PT_RIME)
	    {
		parts[i].vx = 0;
		parts[i].vy = 0;
		for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(rt==PT_SPRK)
                            {
				t = parts[i].type = PT_FOG;
				parts[i].life = rand()%50 + 60;
			    }
			    else if(rt==PT_FOG&&parts[r>>8].life>0)
			    {
				t = parts[i].type = PT_FOG;
				parts[i].life = parts[r>>8].life;
			    }
			}
		}
	    else if(t==PT_DEUT)
	    {
			int maxlife = ((10000/(parts[i].temp + 1))-1);
			if((10000%((int)parts[i].temp+1))>rand()%((int)parts[i].temp+1))
				maxlife ++;
			if(parts[i].life < maxlife)
			{
				for(nx=-1; nx<2; nx++)
				for(ny=-1; ny<2; ny++)
				if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if((r>>8)>=NPART || !r || (parts[i].life >=maxlife))
						continue;
					if(parts[r>>8].type==PT_DEUT&&33>=rand()/(RAND_MAX/100)+1)
					{
						if((parts[i].life + parts[r>>8].life + 1) <= maxlife)
						{
							parts[i].life += parts[r>>8].life + 1;
							parts[r>>8].type=PT_NONE;
						}
					}
				}
			}
			else
			for(nx=-1; nx<2; nx++)
			for(ny=-1; ny<2; ny++)
                 	if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                 	{
						r = pmap[y+ny][x+nx];
						if((r>>8)>=NPART || (parts[i].life<=maxlife))
                    			continue;
						if((bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALLELEC||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_EWALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_DESTROYALL||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_WALL||
							bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWAIR||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWSOLID||bmap[(y+ny)/CELL][(x+nx)/CELL]==WL_ALLOWGAS))
								continue;
                    	 	if((!r)&&parts[i].life>=1)//if nothing then create deut
                    		{
                        		create_part(-1,x+nx,y+ny,PT_DEUT);
                        		parts[i].life--;
					parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;
					parts[pmap[y+ny][x+nx]>>8].life = 0;
                    	 	}
			}
				for( trade = 0; trade<4;trade ++)
				{
					nx = rand()%5-2;
					ny = rand()%5-2;
					if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
					{
						r = pmap[y+ny][x+nx];
						if((r>>8)>=NPART || !r)
							continue;
						if(parts[r>>8].type==t&&(parts[i].life>parts[r>>8].life)&&parts[i].life>0)//diffusion
						{
							int temp = parts[i].life - parts[r>>8].life;
							if(temp ==1)
							{
								parts[r>>8].life ++;
								parts[i].life --;
							}
							else if(temp>0)
							{
								parts[r>>8].life += temp/2;
								parts[i].life -= temp/2;
							}
						}
					}
				}
	    }
	    else if(t==PT_WARP)
			{
				for( trade = 0; trade<5;trade ++)
				{
					nx = rand()%3-1;
					ny = rand()%3-1;
					if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
					{
						r = pmap[y+ny][x+nx];
						if((r>>8)>=NPART || !r)
								continue;
						if(parts[r>>8].type!=PT_WARP&&parts[r>>8].type!=PT_STKM&&parts[r>>8].type!=PT_DMND&&parts[r>>8].type!=PT_CLNE&&parts[r>>8].type!=PT_BCLN&&parts[r>>8].type!=PT_PCLN&&(10>=rand()%200))
						{
							t = parts[i].type = parts[r>>8].type;
							parts[i].ctype = parts[r>>8].ctype;
							parts[i].life = parts[r>>8].life;
							parts[i].tmp = parts[r>>8].tmp;
							parts[i].temp = parts[r>>8].temp;
							parts[i].vx = parts[r>>8].vx;
							parts[i].vy = parts[r>>8].vy;
							parts[r>>8].type = PT_WARP;
							parts[r>>8].life = rand()%90+1;
							trade = 5;
						}
					}
				}				
			}
	    else if(t==PT_FWRK)
	    {
			if((parts[i].temp>400&&(9+parts[i].temp/40)>rand()%100000&&parts[i].life==0&&!pmap[y-1][x])||parts[i].ctype==PT_DUST)
			{
				create_part(-1, x , y-1 , PT_FWRK);
				r = pmap[y-1][x];
				if(parts[r>>8].type==PT_FWRK)
				{
					parts[r>>8].vy = rand()%8-22;
					parts[r>>8].vx = rand()%20-rand()%20;
					parts[r>>8].life=rand()%15+25;
					t=parts[i].type=PT_NONE;
				}
			}
			if(parts[i].life>1)
			{
				if(parts[i].life>=45&&parts[i].type==PT_FWRK)
					parts[i].life=0;
			}
			if((parts[i].life<3&&parts[i].life>0)||parts[i].vy>6&&parts[i].life>0)
			{
				int q = (rand()%255+1);
				int w = (rand()%255+1);
				int e = (rand()%255+1);
				for(nx=-1; nx<2; nx++)
					for(ny=-2; ny<3; ny++)
						if(x+nx>=0 && y+ny>0 &&
							x+nx<XRES && y+ny<YRES)
						{
							if(5>=rand()%8)
							{
								if(!pmap[y+ny][x+nx])
								{
									create_part(-1, x+nx, y+ny , PT_DUST);
									pv[y/CELL][x/CELL] += 2.00f*CFDS;
									a= pmap[y+ny][x+nx];
									if(parts[a>>8].type==PT_DUST) 
									{
										parts[a>>8].vy = -(rand()%10-1);
										parts[a>>8].vx = ((rand()%2)*2-1)*rand()%(5+5)+(parts[i].vx)*2 ;
										parts[a>>8].life= rand()%37+18;
										parts[a>>8].tmp=q;
										parts[a>>8].flags=w;
										parts[a>>8].ctype=e;
										parts[a>>8].temp= rand()%20+6000;
									}
								}
							}
						}
				t=parts[i].type=PT_NONE;
			}
	    }
            else if(t==PT_LCRY)
            {
                for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(rt==PT_SPRK)
                            {
                                if(parts[r>>8].ctype==PT_PSCN)
                                {
                                    parts[i].life = 10;
                                }
                                else if(parts[r>>8].ctype==PT_NSCN)
                                {
                                    parts[i].life = 9;
                                }
                            }
                            if(rt==PT_LCRY)
                            {
                                if(parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
                                {
                                    parts[i].life = 9;
                                }
                                else if(parts[i].life==0&&parts[r>>8].life==10)
                                {
                                    parts[i].life = 10;
                                }
                            }
                        }
            }
	    else if(t==PT_PIPE)
	    {
		    if(!parts[i].ctype && parts[i].life<=10)
		    {
			    if(parts[i].temp<272.15)
			    {
				    if(parts[i].temp>173.25&&parts[i].temp<273.15)
				    {
					    parts[i].ctype = 2;
					    parts[i].life = 0;
				    }
				    if(parts[i].temp>73.25&&parts[i].temp<=173.15)
				    {
					    parts[i].ctype = 3;
					    parts[i].life = 0;
				    }
				    if(parts[i].temp>=0&&parts[i].temp<=73.15)
				    {
					    parts[i].ctype = 4;
					    parts[i].life = 0;
				    }
			    }
			    else
			    {
				for(nx=-2; nx<3; nx++)
				for(ny=-2; ny<3; ny++)
				if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if((r>>8)>=NPART )
						continue;
					if(!r)
						create_part(-1,x+nx,y+ny,PT_BRCK);
				}
				if(parts[i].life==1)
					parts[i].ctype = 1;
			    }
		    }
		    if(parts[i].ctype==1)
		    {
			    for(nx=-1; nx<2; nx++)
				for(ny=-1; ny<2; ny++)
				if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if((r>>8)>=NPART)
						continue;
					if(!r&&!parts[i].life)
						parts[i].life=50;
				}
			    if(parts[i].life==2)
			    {
				    parts[i].ctype = 2;
				    parts[i].life = 6;
			    }
		    }
		    if(parts[i].ctype>1)
		    for( q = 0;q<3;q++)
		    for( ctype = 2;ctype<5;ctype++)
		    {
			if(parts[i].ctype==ctype)
			{
			    if(parts[i].life==3)
			    {
					for(nx=-1; nx<2; nx++)
					for(ny=-1; ny<2; ny++)
					if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
					{
						r = pmap[y+ny][x+nx];
						if((r>>8)>=NPART || !r)
							continue;
						if(parts[r>>8].type==PT_PIPE&&parts[r>>8].ctype==1)
						{
							parts[r>>8].ctype = (((ctype)%3)+2);//reverse
							parts[r>>8].life = 6;
						}
					}
			    }
			    else 
			    {
				nx = rand()%3-1;
				ny = rand()%3-1;
				if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if((r>>8)>=NPART)
						continue;
					else if(!r&&parts[i].tmp!=0)
					{
						if(create_part(-1,x+nx,y+ny,parts[i].tmp))
						{
							parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;//pipe saves temp and life now
							parts[pmap[y+ny][x+nx]>>8].life = parts[i].flags;
						}
						parts[i].tmp = 0;
						continue;
					}
					else if(!r)
						continue;
					else if(parts[i].tmp == 0 && (ptypes[parts[r>>8].type].falldown!= 0 || pstates[parts[r>>8].type].state == ST_GAS))
					{
						parts[i].tmp = parts[r>>8].type;
						parts[i].temp = parts[r>>8].temp;
						parts[i].flags = parts[r>>8].life;
						parts[r>>8].type = PT_NONE;
					}
					else if(parts[r>>8].type==PT_PIPE && parts[r>>8].ctype!=(((ctype)%3)+2) && parts[r>>8].tmp==0&&parts[i].tmp>0)
					{
						parts[r>>8].tmp = parts[i].tmp;
						parts[r>>8].temp = parts[i].temp;
						parts[r>>8].flags = parts[i].flags;
						parts[i].tmp = 0;
					}
				}
			    }
			}
		    }
	    }
	    else if(t==PT_FRZZ)
	    {
		   for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
				if(parts[r>>8].type==PT_WATR&&5>rand()%100)
				{
					parts[r>>8].type=PT_FRZW;
					parts[r>>8].life = 100;
					t = parts[i].type = PT_NONE;
				}
					
			}
	    }
	    else if(t==PT_FRZW)
	    {
		    for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
				if(parts[r>>8].type==PT_WATR&&5>rand()%70)
				{
					parts[r>>8].type=PT_FRZW;
				}
			}
			if(parts[i].life==0&&13>rand()%2500)
			{
				t = parts[i].type=PT_ICEI;
				parts[i].ctype=PT_FRZW;
				parts[i].temp -= 200.0f;
				if(parts[i].temp<0)
					parts[i].temp = 0;
			}
			else if((100-(parts[i].life))>rand()%50000)
			{
				t = parts[i].type=PT_ICEI;
				parts[i].ctype=PT_FRZW;
				parts[i].temp -= 200.0f;
				if(parts[i].temp<0)
					parts[i].temp = 0;
			}
	    }
	    else if(t==PT_INST)
	    {
		    for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny) && abs(nx)+abs(ny)<4)
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    else if(parts[r>>8].type==PT_SPRK&&(parts[r>>8].ctype==PT_PSCN)&&(parts[r>>8].life>=3)&&parts[i].life%4==0)
			    {
				    flood_parts(x,y,PT_SPRK,PT_INST,-1);//add life
				    parts[r>>8].type==parts[r>>8].ctype;
			    }
			}
	    }
	    else if(t==PT_PRTI)
	    {
		    int count =0;
		    for(ny=-1; ny<2; ny++)
                    for(nx=-1; nx<2; nx++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
				count ++;
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    if(parts[r>>8].type==PT_SPRK || (parts[r>>8].type!=PT_PRTI && parts[r>>8].type!=PT_PRTO && (ptypes[parts[r>>8].type].falldown!= 0 || pstates[parts[r>>8].type].state == ST_GAS)))
				    for( nnx=0;nnx<8;nnx++)
					    if(!portal[count-1][nnx])
					    {
						    portal[count-1][nnx] = parts[r>>8].type;
						    portaltemp[count-1][nnx] = parts[r>>8].temp;
						    if(parts[r>>8].type==PT_SPRK)
							parts[r>>8].type = parts[r>>8].ctype;   
						    else 
							parts[r>>8].type = PT_NONE;
						    break;
					    }
			}
	    }
	    else if(t==PT_PRTO)
	    {
		    int count = 0;
		    for(ny=1; ny>-2; ny--)
                    for(nx=1; nx>-2; nx--)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
				count ++;
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || r)
                                continue;
			    if(!r)
			    {
				for( nnx =0 ;nnx<8;nnx++)
				{
					int randomness = count + rand()%3-1;
					if(randomness<1)
						randomness=1;
					if(randomness>8)
						randomness=8;
					if(portal[randomness-1][nnx]==PT_SPRK)//todo. make it look better
					{
						create_part(-1,x+1,y,portal[randomness-1][nnx]);
						create_part(-1,x+1,y+1,portal[randomness-1][nnx]);
						create_part(-1,x+1,y-1,portal[randomness-1][nnx]);
						create_part(-1,x,y-1,portal[randomness-1][nnx]);
						create_part(-1,x,y+1,portal[randomness-1][nnx]);
						create_part(-1,x-1,y+1,portal[randomness-1][nnx]);
						create_part(-1,x-1,y,portal[randomness-1][nnx]);
						create_part(-1,x-1,y-1,portal[randomness-1][nnx]);
						portal[randomness-1][nnx] = 0;
						portaltemp[randomness-1][nnx] = 0;
						break;
					}
					else if(portal[randomness-1][nnx])
					{
						create_part(-1,x+nx,y+ny,portal[randomness-1][nnx]);
						parts[pmap[y+ny][x+nx]>>8].temp = portaltemp[randomness-1][nnx];
						portal[randomness-1][nnx] = 0;
						portaltemp[randomness-1][nnx] = 0;
						break;
					}
				}
			    }
			}
	    }
	    else if(t==PT_WIFI)
	    {
		int temprange = 100;
		for( temp = 0; temp < MAX_TEMP; temp += temprange)
			if(parts[i].temp-73.15>temp&&parts[i].temp-73.15 <temp+temprange)
				parts[i].tmp = temp/100;
		for(ny=-1; ny<2; ny++)
                    for(nx=-1; nx<2; nx++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    if(parts[r>>8].type==PT_NSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
			    {
				    parts[r>>8].type = PT_SPRK;
				    parts[r>>8].ctype = PT_NSCN;
				    parts[r>>8].life = 4;
			    }
			    else if(parts[r>>8].type==PT_PSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
			    {
				    parts[r>>8].type = PT_SPRK;
				    parts[r>>8].ctype = PT_PSCN;
				    parts[r>>8].life = 4;
			    }
			    else if(parts[r>>8].type==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3 && !wireless[parts[i].tmp][0])
			    {
				    //parts[r>>8].type = parts[r>>8].ctype;
				    wireless[parts[i].tmp][0] = 1;
				    wireless[parts[i].tmp][1] = 1;
				    ISWIRE = 1;
			    }
			}
	    }
            else if(t==PT_PCLN)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(rt==PT_SPRK)
                            {
                                if(parts[r>>8].ctype==PT_PSCN)
                                {
                                    parts[i].life = 10;
                                }
                                else if(parts[r>>8].ctype==PT_NSCN)
                                {
                                    parts[i].life = 9;
                                }
                            }
                            if(rt==PT_PCLN)
                            {
                                if(parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
                                {
                                    parts[i].life = 9;
                                }
                                else if(parts[i].life==0&&parts[r>>8].life==10)
                                {
                                    parts[i].life = 10;
                                }
                            }
                        }
            }
            else if(t==PT_HSWC)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(rt==PT_SPRK)
                            {
                                if(parts[r>>8].ctype==PT_PSCN)
                                {
                                    parts[i].life = 10;
                                }
                                else if(parts[r>>8].ctype==PT_NSCN)
                                {
                                    parts[i].life = 9;
                                }
                            }
                            if(rt==PT_HSWC)
                            {
                                if(parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
                                {
                                    parts[i].life = 9;
                                }
                                else if(parts[i].life==0&&parts[r>>8].life==10)
                                {
                                    parts[i].life = 10;
                                }
                            }
                        }
            }
	    else if(t==PT_PUMP)
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(rt==PT_SPRK)
                            {
                                if(parts[r>>8].ctype==PT_PSCN)
                                {
                                    parts[i].life = 10;
                                }
                                else if(parts[r>>8].ctype==PT_NSCN)
                                {
                                    parts[i].life = 9;
                                }
                            }
                            if(rt==PT_PUMP)
                            {
                                if(parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
                                {
                                    parts[i].life = 9;
                                }
                                else if(parts[i].life==0&&parts[r>>8].life==10)
                                {
                                    parts[i].life = 10;
                                }
                            }
                        }
            }
	    else if(t==PT_SHLD1)
	    {
		    for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
			    else if(parts[r>>8].type==PT_SPRK&&parts[i].life==0)
				for( nnx=-1;nnx<2;nnx++)
					for( nny=-1;nny<2;nny++)
					{
						if(7>rand()%200&&parts[i].life==0)
						{
							t = parts[i].type = PT_SHLD2;
							parts[i].life = 7;
						}
						else if(!pmap[y+ny+nny][x+nx+nnx])
						{
							create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
							//parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
						}
					}
			    else if(parts[r>>8].type==PT_SHLD3&&4>rand()%10)
			    {
				    t = parts[i].type=PT_SHLD2;
				    parts[i].life = 7;
			    }
			}
	    }
	    else if(t==PT_SHLD2)
	    {
		    for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART)
                                continue;
			    if(!r && parts[i].life>0)
				    create_part(-1,x+nx,y+ny,PT_SHLD1);
			    if(!r)
					continue;
			    else if(parts[r>>8].type==PT_SPRK&&parts[i].life==0)
				for( nnx=-1;nnx<2;nnx++)
					for( nny=-1;nny<2;nny++)
					{
						if(3>rand()%200&&parts[i].life==0)
						{
							t = parts[i].type = PT_SHLD3;
							parts[i].life = 7;
						}
						else if(!pmap[y+ny+nny][x+nx+nnx])
						{
						create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
						parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
						}
					}
			    else if(parts[r>>8].type==PT_SHLD4&&4>rand()%10)
			    {
				    t = parts[i].type=PT_SHLD3;
				    parts[i].life = 7;
			    }
			}
	    }
	    else if(t==PT_SHLD3)
	    {
		    for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART)
                                continue;
			if(!r)
			{
				if(1>rand()%2500)
				{
					create_part(-1,x+nx,y+ny,PT_SHLD1);
					parts[pmap[y+ny][x+nx]>>8].life=7;
					t = parts[i].type = PT_SHLD2;
				}
				else
					continue;
				
			}
			if(parts[r>>8].type==PT_SHLD1 && parts[i].life>3)
			{
				    parts[r>>8].type = PT_SHLD2;
				    parts[r>>8].life=7;
			}
			    else if(parts[r>>8].type==PT_SPRK&&parts[i].life==0)
				for( nnx=-1;nnx<2;nnx++)
					for( nny=-1;nny<2;nny++)
					{
						if(2>rand()%3000&&parts[i].life==0)
						{
							t = parts[i].type = PT_SHLD4;
							parts[i].life = 7;
						}
						else if(!pmap[y+ny+nny][x+nx+nnx])
						{
						create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
						parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
						}
					}
			}
	    }
	    else if(t==PT_SHLD4)
	    {
		    for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART)
                                continue;
			if(!r)
			{
				if(1>rand()%5500)
				{
					create_part(-1,x+nx,y+ny,PT_SHLD1);
					parts[pmap[y+ny][x+nx]>>8].life=7;
					t = parts[i].type = PT_SHLD2;
				}
				else
					continue;
				
			}
			if(parts[r>>8].type==PT_SHLD2 && parts[i].life>3)
			{
				    parts[r>>8].type = PT_SHLD3;
				    parts[r>>8].life = 7;
			}
			    else if(parts[r>>8].type==PT_SPRK&&parts[i].life==0)
				for( nnx=-1;nnx<2;nnx++)
					for( nny=-1;nny<2;nny++)
					{
						if(!pmap[y+ny+nny][x+nx+nnx])
						{
						create_part(-1,x+nx+nnx,y+ny+nny,PT_SHLD1);
						parts[pmap[y+ny+nny][x+nx+nnx]>>8].life=7;
						}
					}
			}
	    }
            else if(t==PT_AMTR)
            {
                for(nx=-1; nx<2; nx++)
                    for(ny=-1; ny<2; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if((r&0xFF)!=PT_AMTR && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_NONE && (r&0xFF)!=PT_PHOT && (r&0xFF)!=PT_VOID && (r&0xFF)!=PT_BHOL)
                            {
                                t = parts[i].life++;
                                if(parts[i].life==3)
                                {
                                    parts[i].type = PT_NONE;
                                    kill_part(i);
				    goto killed;
                                }
                                parts[r>>8].life = 0;
                                parts[r>>8].type = PT_NONE;
                                kill_part(r>>8);
                                if(2>(rand()/(RAND_MAX/100)))
                                    create_part(-1, x+nx, y+ny, PT_PHOT);
                                pv[y/CELL][x/CELL] -= 5.0f;
                                //goto killed;
                            }
                        }
            }
            else if(t==PT_FIRW) {
                if(parts[i].tmp==0) {
                    for(nx=-1; nx<2; nx++)
                        for(ny=-1; ny<2; ny++)
                            if(x+nx>=0 && y+ny>0 &&
                                    x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                rt = parts[r>>8].type;
                                if(rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
                                {
                                    parts[i].tmp = 1;
                                    parts[i].life = rand()%50+60;
                                }
                            }
                }
                else if(parts[i].tmp==1) {
                    if(parts[i].life==0) {
                        parts[i].tmp=2;
                    } else {
                        float newVel = parts[i].life/25;
                        parts[i].flags = parts[i].flags&0xFFFFFFFE;
                        if((pmap[(int)(ly-newVel)][(int)lx]&0xFF)==PT_NONE && ly-newVel>0) {
                            parts[i].vy = -newVel;
                            ly-=newVel;
                            iy-=newVel;
                        }
                    }
                }
                else if(parts[i].tmp==2) {
                    int col = rand()%200+4;
                    for(nx=-2; nx<3; nx++) {
                        for(ny=-2; ny<3; ny++) {
                            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                int tmul = rand()%7;
                                create_part(-1, x+nx, y+ny, PT_FIRW);
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                if(parts[r>>8].type==PT_FIRW) {
                                    parts[r>>8].vx = (rand()%3-1)*tmul;
                                    parts[r>>8].vy = (rand()%3-1)*tmul;
                                    parts[r>>8].tmp = col;
                                    parts[r>>8].life = rand()%100+100;
                                    parts[r>>8].temp = 6000.0f;
                                }
                            }
                        }
                    }
                    pv[y/CELL][x/CELL] += 20;
                    kill_part(i);
					goto killed;
                } else if(parts[i].tmp>=3) {
                    if(parts[i].life<=0) {
                        kill_part(i);
						goto killed;
                    }
                }
            }
            else if(t==PT_BTRY)
            {
                rt = 3 + (int)pv[y/CELL][x/CELL];
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(parts_avg(i,r>>8,PT_INSL) != PT_INSL)
                            {
                                if((rt==PT_METL||rt==PT_IRON||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN||rt==PT_NBLE)&&parts[r>>8].life==0 && abs(nx)+abs(ny) < 4)
                                {
                                    parts[r>>8].life = 4;
                                    parts[r>>8].ctype = rt;
                                    parts[r>>8].type = PT_SPRK;
                                }
                            }
                        }
            } else if(t==PT_SWCH)
            {
                rt = 3 + (int)pv[y/CELL][x/CELL];
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            rt = parts[r>>8].type;
                            if(parts[r>>8].type == PT_SWCH&&parts_avg(i,r>>8,PT_INSL)!=PT_INSL)
                            {
                                if(parts[i].life==10&&parts[r>>8].life<10&&parts[r>>8].life>0)
                                {
                                    parts[i].life = 9;
                                }
                                else if(parts[i].life==0&&parts[r>>8].life==10)
                                {
                                    parts[i].life = 10;
                                }
                            }
                        }
            }
            if(t==PT_SWCH)
                if((parts[i].life>0&&parts[i].life<10)|| parts[i].life > 10)
                {
                    parts[i].life--;
                }
            if(t==PT_FIRE || t==PT_PLSM || t==PT_LAVA || t==PT_SPRK || fe || (t==PT_PHOT&&(1>rand()%10)))
            {
                for(nx=-2; nx<3; nx++)
                    for(ny=-2; ny<3; ny++)
                        if(x+nx>=0 && y+ny>0 &&
                                x+nx<XRES && y+ny<YRES && (nx || ny))
                        {
                            r = pmap[y+ny][x+nx];
                            if((r>>8)>=NPART || !r)
                                continue;
                            if(bmap[(y+ny)/CELL][(x+nx)/CELL] && bmap[(y+ny)/CELL][(x+nx)/CELL]!=WL_STREAM)
                                continue;
                            rt = parts[r>>8].type;
                            if((a || ptypes[rt].explosive) && ((rt!=PT_RBDM && rt!=PT_LRBD && rt!=PT_INSL && rt!=PT_SWCH) || t!=PT_SPRK) &&
                                    !(t==PT_PHOT && rt==PT_INSL) &&
                                    (t!=PT_LAVA || parts[i].life>0 || (rt!=PT_STNE && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_METL  && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SWCH && rt!=PT_INWR)) && !(rt==PT_CRAC && parts[r>>8].life>0) &&
                                    ptypes[rt].flammable && (ptypes[rt].flammable + (int)(pv[(y+ny)/CELL][(x+nx)/CELL]*10.0f))>(rand()%1000))
                            {
                                parts[r>>8].type = PT_FIRE;
                                parts[r>>8].temp = ptypes[PT_FIRE].heat + (ptypes[rt].flammable/2);
                                parts[r>>8].life = rand()%80+180;
                                if(ptypes[rt].explosive)
                                    pv[y/CELL][x/CELL] += 0.25f * CFDS;
                                continue;
                            }
                            lpv = (int)pv[(y+ny)/CELL][(x+nx)/CELL];
                            if(lpv < 1) lpv = 1;
                            if(legacy_enable)
                            {
                                if(t!=PT_SPRK && ptypes[rt].meltable  && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_IRON && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) &&
                                        ptypes[rt].meltable*lpv>(rand()%1000))
                                {
                                    if(t!=PT_LAVA || parts[i].life>0)
                                    {
                                        parts[r>>8].ctype = (parts[r>>8].type==PT_BRMT)?PT_BMTL:parts[r>>8].type;
                                        parts[r>>8].ctype = (parts[r>>8].ctype==PT_SAND)?PT_GLAS:parts[r>>8].ctype;
                                        parts[r>>8].type = PT_LAVA;
                                        parts[r>>8].life = rand()%120+240;
                                    }
                                    else
                                    {
                                        parts[i].life = 0;
                                        t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
                                        parts[i].ctype = PT_NONE;//rt;
                                        goto killed;
                                    }
                                }
                                if(t!=PT_SPRK && (rt==PT_ICEI || rt==PT_SNOW))
                                {
                                    parts[r>>8].type = PT_WATR;
                                    if(t==PT_FIRE)
                                    {
                                        parts[i].x = lx;
                                        parts[i].y = ly;
                                        kill_part(i);
                                        goto killed;
                                    }
                                    if(t==PT_LAVA)
                                    {
                                        parts[i].life = 0;
                                        t = parts[i].type = PT_STNE;
                                        goto killed;
                                    }
                                }
                                if(t!=PT_SPRK && (rt==PT_WATR || rt==PT_DSTW || rt==PT_SLTW))
                                {
                                    kill_part(r>>8);
                                    if(t==PT_FIRE)
                                    {
                                        parts[i].x = lx;
                                        parts[i].y = ly;
                                        kill_part(i);
                                        goto killed;
                                    }
                                    if(t==PT_LAVA)
                                    {
                                        parts[i].life = 0;
                                        t = parts[i].type = (parts[i].ctype)?parts[i].ctype:PT_STNE;
                                        parts[i].ctype = PT_NONE;
                                        goto killed;
                                    }
                                }
                            }
                            //Check if there is a SWCH that is currently covered with SPRK
                            //If so check if the current SPRK is covering a NSCN
                            //If so turn the SPRK that covers the SWCH back into SWCH and turn it off
                            if(rt==PT_SPRK && parts[r>>8].ctype == PT_SWCH && t==PT_SPRK)
                            {
                                pavg = parts_avg(r>>8, i,PT_INSL);
                                if(parts[i].ctype == PT_NSCN&&pavg != PT_INSL)
                                {
                                    parts[r>>8].type = PT_SWCH;
                                    parts[r>>8].ctype = PT_NONE;
                                    parts[r>>8].life = 9;
                                }
                            }
                            pavg = parts_avg(i, r>>8,PT_INSL);
                            if(rt==PT_SWCH && t==PT_SPRK)
                            {
                                pavg = parts_avg(r>>8, i,PT_INSL);
                                if(parts[i].ctype == PT_PSCN&&pavg != PT_INSL && parts[r>>8].life<10)
                                    parts[r>>8].life = 10;
                                if(parts[i].ctype == PT_NSCN&&pavg != PT_INSL)
                                    parts[r>>8].life = 9;
                                if(!(parts[i].ctype == PT_PSCN||parts[i].ctype == PT_NSCN)&&parts[r>>8].life == 10&&pavg != PT_INSL)
                                {
                                    parts[r>>8].type = PT_SPRK;
                                    parts[r>>8].ctype = PT_SWCH;
                                    parts[r>>8].life = 4;
                                }
                            }
                            pavg = parts_avg(i, r>>8,PT_INSL);
                            if(pavg != PT_INSL)
                            {
                                if(t==PT_SPRK && (rt==PT_METL||rt==PT_IRON||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN||rt==PT_NBLE) && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if(!(rt==PT_PSCN&&parts[i].ctype==PT_NSCN)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN&&parts[i].temp>=373.0f)&&parts[i].ctype==PT_NTCT)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN&&parts[i].temp<=373.0f)&&parts[i].ctype==PT_PTCT)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN)&&parts[i].ctype==PT_INWR) && pavg != PT_INSL &&!(parts[i].ctype==PT_SWCH&&(rt==PT_PSCN||rt==PT_NSCN))&&!(parts[i].ctype==PT_INST&&rt!=PT_NSCN) )
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                        if(parts[r>>8].temp+10.0f<673.0f&&!legacy_enable&&!(rt==PT_LRBD||rt==PT_RBDM||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR))
                                            parts[r>>8].temp = parts[r>>8].temp+10.0f;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_NTCT && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if((parts[i].ctype==PT_NSCN||parts[i].ctype==PT_NTCT||(parts[i].ctype==PT_PSCN&&parts[r>>8].temp>373.0f))&&pavg != PT_INSL)
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_PTCT && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if((parts[i].ctype==PT_NSCN||parts[i].ctype==PT_PTCT||(parts[i].ctype==PT_PSCN&&parts[r>>8].temp<373.0f))&&pavg != PT_INSL)
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_INWR && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if((parts[i].ctype==PT_NSCN||parts[i].ctype==PT_INWR||parts[i].ctype==PT_PSCN)&&pavg != PT_INSL)
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_WATR && parts[r>>8].life==0 &&
                                        (parts[i].life<2 || ((r>>8)<i && parts[i].life<3)) && abs(nx)+abs(ny)<4)
                                {
                                    parts[r>>8].type = PT_SPRK;
                                    parts[r>>8].life = 6;
                                    parts[r>>8].ctype = rt;
                                }
                                if(t==PT_SPRK && rt==PT_SLTW && parts[r>>8].life==0 &&
                                        (parts[i].life<2 || ((r>>8)<i && parts[i].life<3)) && abs(nx)+abs(ny)<4)
                                {
                                    parts[r>>8].type = PT_SPRK;
                                    parts[r>>8].life = 5;
                                    parts[r>>8].ctype = rt;
                                }
                                if(t==PT_SPRK&&parts[i].ctype==PT_ETRD&&parts[i].life==5)
                                {
                                    if(rt==PT_METL||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN)
                                    {
                                        t = parts[i].type = PT_ETRD;
                                        parts[i].ctype = PT_NONE;
                                        parts[i].life = 20;
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                    }
                                }

                                if(t==PT_SPRK&&parts[i].ctype==PT_NBLE&&parts[i].life<=1)
                                {
                                    parts[i].life = rand()%150+50;
                                    parts[i].type = PT_PLSM;
                                    parts[i].ctype = PT_NBLE;
                                    parts[i].temp = 3500;
                                    pv[y/CELL][x/CELL] += 1;
                                }
                                if(t==PT_SPRK&&parts[i].ctype==PT_SWCH&&parts[i].life<=0)
                                {
                                    parts[i].type = PT_SWCH;
                                    parts[i].life = 14;
                                }
                            }
                        }
killed:
                if(parts[i].type == PT_NONE)
                    continue;
            }
            if(t==PT_STKM)
            {
                float dt = 0.9;///(FPSB*FPSB);  //Delta time in square
                //Tempirature handling
                if(parts[i].temp<243)
                    parts[i].life -= 1;
                if((parts[i].temp<309.6f) && (parts[i].temp>=243))
                    parts[i].temp += 1;

                //Death
                if(parts[i].life<1 || death == 1 || (pv[y/CELL][x/CELL]>=4.5f && player[2] != SPC_AIR) )  //If his HP is less that 0 or there is very big wind...
                {
                    death = 0;
                    for(r=-2; r<=1; r++)
                    {
                        create_part(-1, x+r, y-2, player[2]);
                        create_part(-1, x+r+1, y+2, player[2]);
                        create_part(-1, x-2, y+r+1, player[2]);
                        create_part(-1, x+2, y+r, player[2]);
                    }
                    kill_part(i);  //Kill him
                    goto killed;
                }

                parts[i].vy += -0.7*dt;  //Head up!

                //Verlet integration
                pp = 2*player[3]-player[5]+player[19]*dt*dt;;
                player[5] = player[3];
                player[3] = pp;
                pp = 2*player[4]-player[6]+player[20]*dt*dt;;
                player[6] = player[4];
                player[4] = pp;

                pp = 2*player[7]-player[9]+player[21]*dt*dt;;
                player[9] = player[7];
                player[7] = pp;
                pp = 2*player[8]-player[10]+(player[22]+1)*dt*dt;;
                player[10] = player[8];
                player[8] = pp;

                pp = 2*player[11]-player[13]+player[23]*dt*dt;;
                player[13] = player[11];
                player[11] = pp;
                pp = 2*player[12]-player[14]+player[24]*dt*dt;;
                player[14] = player[12];
                player[12] = pp;

                pp = 2*player[15]-player[17]+player[25]*dt*dt;;
                player[17] = player[15];
                player[15] = pp;
                pp = 2*player[16]-player[18]+(player[26]+1)*dt*dt;;
                player[18] = player[16];
                player[16] = pp;

                //Setting acceleration to 0
                player[19] = 0;
                player[20] = 0;

                player[21] = 0;
                player[22] = 0;

                player[23] = 0;
                player[24] = 0;

                player[25] = 0;
                player[26] = 0;

                //Go left
                if (((int)(player[0])&0x01) == 0x01 && pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_GAS)
                {
                    if (pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_LIQUID
                            && (pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF) != PT_LNTG)
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[21] = -3;
                            player[22] = -2;
                            player[19] = -2;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[25] = -3;
                            player[26] = -2;
                            player[23] = -2;
                        }
                    }
                    else
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])  //It should move another way in liquids
                        {
                            player[21] = -1;
                            player[22] = -1;
                            player[19] = -1;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[25] = -1;
                            player[26] = -1;
                            player[23] = -1;
                        }
                    }
                }

                //Go right
                if (((int)(player[0])&0x02) == 0x02 && pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_GAS)
                {
                    if (pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_LIQUID
                            && (pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF) != PT_LNTG)
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[21] = 3;
                            player[22] = -2;
                            player[19] = 2;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[25] = 3;
                            player[26] = -2;
                            player[23] = 2;
                        }
                    }
                    else
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[21] = 1;
                            player[22] = -1;
                            player[19] = 1;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[25] = 1;
                            player[26] = -1;
                            player[23] = 1;
                        }

                    }
                }

                //Jump
                if (((int)(player[0])&0x04) == 0x04 && (pstates[pmap[(int)(player[8]-0.5)][(int)(player[7])]&0xFF].state != ST_GAS || pstates[pmap[(int)(player[16]-0.5)][(int)(player[15])]&0xFF].state != ST_GAS))
                {
                    if (pmap[(int)(player[8]-0.5)][(int)(player[7])] || pmap[(int)(player[16]-0.5)][(int)(player[15])])
                    {
                        parts[i].vy = -5;
                        player[22] -= 1;
                        player[26] -= 1;
                    }
                }

                //Charge detector wall if foot inside
                if(bmap[(int)(player[8]+0.5)/CELL][(int)(player[7]+0.5)/CELL]==WL_DETECT)
                    set_emap((int)player[7]/CELL, (int)player[8]/CELL);
                if(bmap[(int)(player[16]+0.5)/CELL][(int)(player[15]+0.5)/CELL]==WL_DETECT)
                    set_emap((int)(player[15]+0.5)/CELL, (int)(player[16]+0.5)/CELL);

                //Searching for particles near head
                for(nx = -2; nx <= 2; nx++)
                    for(ny = 0; ny>=-2; ny--)
                    {
                        if(!pmap[ny+y][nx+x] || (pmap[ny+y][nx+x]>>8)>=NPART)
                            continue;
                        if(ptypes[pmap[ny+y][nx+x]&0xFF].falldown!=0 || (pmap[ny+y][nx+x]&0xFF) == PT_NEUT || (pmap[ny+y][nx+x]&0xFF) == PT_PHOT)
                        {
                            player[2] = pmap[ny+y][nx+x]&0xFF;  //Current element
                        }
                        if((pmap[ny+y][nx+x]&0xFF) == PT_PLNT && parts[i].life<100)  //Plant gives him 5 HP
                        {
                            if(parts[i].life<=95)
                                parts[i].life += 5;
                            else
                                parts[i].life = 100;
                            kill_part(pmap[ny+y][nx+x]>>8);
                        }

                        if((pmap[ny+y][nx+x]&0xFF) == PT_NEUT)
                        {
                            parts[i].life -= (102-parts[i].life)/2;
                            kill_part(pmap[ny+y][nx+x]>>8);
                        }
                        if(bmap[(ny+y)/CELL][(nx+x)/CELL]==WL_FAN)
                            player[2] = SPC_AIR;
                    }

                //Head position
                nx = x + 3*((((int)player[1])&0x02) == 0x02) - 3*((((int)player[1])&0x01) == 0x01);
                ny = y - 3*(player[1] == 0);

                //Spawn
                if(((int)(player[0])&0x08) == 0x08)
                {
                    ny -= 2*(rand()%2)+1;
                    r = pmap[ny][nx];
                    if(!((r>>8)>=NPART))
                    {
                        if(pstates[r&0xFF].state == ST_SOLID)
                        {
                            create_part(-1, nx, ny, PT_SPRK);
                        }
                        else
                        {
                            if(player[2] == SPC_AIR)
                                create_parts(nx + 3*((((int)player[1])&0x02) == 0x02) - 3*((((int)player[1])&0x01) == 0x01), ny, 4, 4, SPC_AIR);
                            else
                                create_part(-1, nx, ny, player[2]);

                            r = pmap[ny][nx];
                            if( ((r>>8) < NPART) && (r>>8)>=0 && player[2] != PT_PHOT && player[2] != SPC_AIR)
                                parts[r>>8].vx = parts[r>>8].vx + 5*((((int)player[1])&0x02) == 0x02) - 5*(((int)(player[1])&0x01) == 0x01);
                            if(((r>>8) < NPART) && (r>>8)>=0 && player[2] == PT_PHOT)
                            {
                                int random = abs(rand()%3-1)*3;
                                if (random==0)
                                {
                                    parts[r>>8].life = 0;
                                    parts[r>>8].type = PT_NONE;
                                }
                                else
                                {
                                    parts[r>>8].vy = 0;
                                    parts[r>>8].vx = (((((int)player[1])&0x02) == 0x02) - (((int)(player[1])&0x01) == 0x01))*random;
                                }
                            }

                        }
                    }
                }

                //Simulation of joints
                d = 25/(pow((player[3]-player[7]), 2) + pow((player[4]-player[8]), 2)+25) - 0.5;  //Fast distance
                player[7] -= (player[3]-player[7])*d;
                player[8] -= (player[4]-player[8])*d;
                player[3] += (player[3]-player[7])*d;
                player[4] += (player[4]-player[8])*d;

                d = 25/(pow((player[11]-player[15]), 2) + pow((player[12]-player[16]), 2)+25) - 0.5;
                player[15] -= (player[11]-player[15])*d;
                player[16] -= (player[12]-player[16])*d;
                player[11] += (player[11]-player[15])*d;
                player[12] += (player[12]-player[16])*d;

                d = 36/(pow((player[3]-parts[i].x), 2) + pow((player[4]-parts[i].y), 2)+36) - 0.5;
                parts[i].vx -= (player[3]-parts[i].x)*d;
                parts[i].vy -= (player[4]-parts[i].y)*d;
                player[3] += (player[3]-parts[i].x)*d;
                player[4] += (player[4]-parts[i].y)*d;

                d = 36/(pow((player[11]-parts[i].x), 2) + pow((player[12]-parts[i].y), 2)+36) - 0.5;
                parts[i].vx -= (player[11]-parts[i].x)*d;
                parts[i].vy -= (player[12]-parts[i].y)*d;
                player[11] += (player[11]-parts[i].x)*d;
                player[12] += (player[12]-parts[i].y)*d;

                //Side collisions checking
                for(nx = -3; nx <= 3; nx++)
                {
                    r = pmap[(int)(player[16]-2)][(int)(player[15]+nx)];
                    if(r && pstates[r&0xFF].state != ST_GAS && pstates[r&0xFF].state != ST_LIQUID)
                        player[15] -= nx;

                    r = pmap[(int)(player[8]-2)][(int)(player[7]+nx)];
                    if(r && pstates[r&0xFF].state != ST_GAS && pstates[r&0xFF].state != ST_LIQUID)
                        player[7] -= nx;
                }

                //Collision checks
                for(ny = -2-(int)parts[i].vy; ny<=0; ny++)
                {
                    r = pmap[(int)(player[8]+ny)][(int)(player[7]+0.5)];  //This is to make coding more pleasant :-)

                    //For left leg
                    if (r && (r&0xFF)!=PT_STKM)
                    {
                        if(pstates[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG)  //Liquid checks  //Liquid checks
                        {
                            if(parts[i].y<(player[8]-10))
                                parts[i].vy = 1*dt;
                            else
                                parts[i].vy = 0;
                            if(abs(parts[i].vx)>1)
                                parts[i].vx *= 0.5*dt;
                        }
                        else
                        {
                            if(pstates[r&0xFF].state != ST_GAS)
                            {
                                player[8] += ny-1;
                                parts[i].vy -= 0.5*parts[i].vy*dt;
                            }
                        }
                        player[9] = player[7];
                    }

                    r = pmap[(int)(player[16]+ny)][(int)(player[15]+0.5)];

                    //For right leg
                    if (r && (r&0xFF)!=PT_STKM)
                    {
                        if(pstates[r&0xFF].state == ST_LIQUID || (r&0xFF) == PT_LNTG)
                        {
                            if(parts[i].y<(player[16]-10))
                                parts[i].vy = 1*dt;
                            else
                                parts[i].vy = 0;
                            if(abs(parts[i].vx)>1)
                                parts[i].vx *= 0.5*dt;
                        }
                        else
                        {
                            if(pstates[r&0xFF].state != ST_GAS)
                            {
                                player[16] += ny-1;
                                parts[i].vy -= 0.5*parts[i].vy*dt;
                            }
                        }
                        player[17] = player[15];
                    }

                    //If it falls too fast
                    if (parts[i].vy>=30)
                    {
                        parts[i].y -= (10+ny)*dt;
                        parts[i].vy = -10*dt;
                    }

                }

                //Keeping legs distance
                if (pow((player[7] - player[15]), 2)<16 && pow((player[8]-player[16]), 2)<1)
                {
                    player[21] -= 0.2;
                    player[25] += 0.2;
                }

                if (pow((player[3] - player[11]), 2)<16 && pow((player[4]-player[12]), 2)<1)
                {
                    player[19] -= 0.2;
                    player[23] += 0.2;
                }

                //If legs touch something
                r = pmap[(int)(player[8]+0.5)][(int)(player[7]+0.5)];
                if((r&0xFF)==PT_SPRK && r && (r>>8)<NPART)  //If on charge
                {
                    parts[i].life -= (int)(rand()/1000)+38;
                }

                if (r>0 && (r>>8)<NPART)  //If hot or cold
                {
                    if(parts[r>>8].temp>=323 || parts[r>>8].temp<=243)
                    {
                        parts[i].life -= 2;
                        player[26] -= 1;
                    }
                }

                if ((r&0xFF)==PT_ACID)  //If on acid
                    parts[i].life -= 5;

                if ((r&0xFF)==PT_PLUT)  //If on plut
                    parts[i].life -= 1;

                r = pmap[(int)(player[16]+0.5)][(int)(player[15]+0.5)];
                if((r&0xFF)==PT_SPRK && r && (r>>8)<NPART)  //If on charge
                {
                    parts[i].life -= (int)(rand()/1000)+38;
                }

                if(r>0 && (r>>8)<NPART)  //If hot or cold
                {
                    if(parts[r>>8].temp>=323 || parts[r>>8].temp<=243)
                    {
                        parts[i].life -= 2;
                        player[22] -= 1;
                    }
                }

                if ((r&0xFF)==PT_ACID)  //If on acid
                    parts[i].life -= 5;

                if ((r&0xFF)==PT_PLUT)  //If on plut
                    parts[i].life -= 1;

                isplayer = 1;
            }
            if(t==PT_CLNE)
            {
                if(!parts[i].ctype)
                {
                    for(nx=-1; nx<2; nx++)
                        for(ny=-1; ny<2; ny++)
                            if(x+nx>=0 && y+ny>0 &&
                                    x+nx<XRES && y+ny<YRES &&
                                    pmap[y+ny][x+nx] &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_CLNE &&
				    (pmap[y+ny][x+nx]&0xFF)!=PT_BCLN &&
				    (pmap[y+ny][x+nx]&0xFF)!=PT_PCLN &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_STKM &&
                                    (pmap[y+ny][x+nx]&0xFF)!=0xFF)
                                parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
                }
                else {
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
                }

            }
	    if(t==PT_BCLN)
            {
                if(!parts[i].ctype)
                {
                    for(nx=-1; nx<2; nx++)
                        for(ny=-1; ny<2; ny++)
                            if(x+nx>=0 && y+ny>0 &&
                                    x+nx<XRES && y+ny<YRES &&
                                    pmap[y+ny][x+nx] &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_CLNE &&
				    (pmap[y+ny][x+nx]&0xFF)!=PT_BCLN &&
				    (pmap[y+ny][x+nx]&0xFF)!=PT_PCLN &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_STKM &&
                                    (pmap[y+ny][x+nx]&0xFF)!=0xFF)
                                parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
                }
                else {
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
                }

            }
            if(parts[i].type==PT_PCLN)
            {
                if(!parts[i].ctype)
                    for(nx=-1; nx<2; nx++)
                        for(ny=-1; ny<2; ny++)
                            if(x+nx>=0 && y+ny>0 &&
                                    x+nx<XRES && y+ny<YRES &&
                                    pmap[y+ny][x+nx] &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_CLNE &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_PCLN &&
				    (pmap[y+ny][x+nx]&0xFF)!=PT_BCLN &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_SPRK &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_NSCN &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_PSCN &&
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_STKM &&
                                    (pmap[y+ny][x+nx]&0xFF)!=0xFF)
                                parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
                if(parts[i].ctype && parts[i].life==10) {
                    if(parts[i].ctype==PT_PHOT) {
                        for(nx=-1; nx<2; nx++) {
                            for(ny=-1; ny<2; ny++) {
                                r = create_part(-1, x+nx, y+ny, parts[i].ctype);
                                if(r!=-1) {
                                    parts[r].vx = nx*3;
                                    parts[r].vy = ny*3;
                                }
                            }
                        }
                    }
		    else if(parts[i].ctype>77&&parts[i].ctype<77+NGOL) {
                        for(nx=-1; nx<2; nx++) {
                            for(ny=-1; ny<2; ny++) {
                                create_part(-1, x+nx, y+ny, parts[i].ctype);
                            }
                        }
                    }else {
                        create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
                    }
                }
            }
            if(t==PT_YEST)
            {
                if(parts[i].temp>303&&parts[i].temp<317) {
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_YEST);
                }
            }
            if(t==PT_PLSM&&parts[i].ctype == PT_NBLE&&parts[i].life <=1)
            {
                parts[i].type = PT_NBLE;
                parts[i].life = 0;
            }
            if (t==PT_FIRE && parts[i].life <=1 && parts[i].temp<625)
            {
                t = parts[i].type = PT_SMKE;
                parts[i].life = rand()%20+250;
            }

            nx = (int)(parts[i].x+0.5f);
            ny = (int)(parts[i].y+0.5f);

            if(nx<CELL || nx>=XRES-CELL ||
                    ny<CELL || ny>=YRES-CELL)
            {
                parts[i].x = lx;
                parts[i].y = ly;
                kill_part(i);
                continue;
            }

            if(parts[i].type == PT_PHOT) {
                rt = pmap[ny][nx] & 0xFF;

                if(rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN) {
                    lt = pmap[ny][nx] >> 8;
                    if(!parts[lt].ctype)
                        parts[lt].ctype = PT_PHOT;
                }

                lt = pmap[y][x] & 0xFF;

                r = eval_move(PT_PHOT, nx, ny, NULL);

                if(((rt==PT_GLAS && lt!=PT_GLAS) || (rt!=PT_GLAS && lt==PT_GLAS)) && r) {
                    if(!get_normal_interp(REFRACT|parts[i].type, x, y, parts[i].vx, parts[i].vy, &nrx, &nry)) {
                        kill_part(i);
                        continue;
                    }

                    r = get_wavelength_bin(&parts[i].ctype);
                    if(r == -1) {
                        kill_part(i);
                        continue;
                    }
                    nn = GLASS_IOR - GLASS_DISP*(r-15)/15.0f;
                    nn *= nn;

                    nrx = -nrx;
                    nry = -nry;
                    if(rt==PT_GLAS && lt!=PT_GLAS)
                        nn = 1.0f/nn;
                    ct1 = parts[i].vx*nrx + parts[i].vy*nry;
                    ct2 = 1.0f - (nn*nn)*(1.0f-(ct1*ct1));
                    if(ct2 < 0.0f) {
                        parts[i].vx -= 2.0f*ct1*nrx;
                        parts[i].vy -= 2.0f*ct1*nry;
                        parts[i].x = lx;
                        parts[i].y = ly;
                        nx = (int)(lx + 0.5f);
                        ny = (int)(ly + 0.5f);
                    } else {
                        ct2 = sqrtf(ct2);
                        ct2 = ct2 - nn*ct1;
                        parts[i].vx = nn*parts[i].vx + ct2*nrx;
                        parts[i].vy = nn*parts[i].vy + ct2*nry;
                    }
                }
            }

            rt = parts[i].flags & FLAG_STAGNANT;
            parts[i].flags &= ~FLAG_STAGNANT;
            if(!try_move(i, x, y, nx, ny))
            {
                parts[i].x = lx;
                parts[i].y = ly;
                if(ptypes[t].falldown)
                {
                    if(nx!=x && try_move(i, x, y, nx, y))
                    {
                        parts[i].x = ix;
                        parts[i].vx *= ptypes[t].collision;
                        parts[i].vy *= ptypes[t].collision;
                    }
                    else if(ny!=y && try_move(i, x, y, x, ny))
                    {
                        parts[i].y = iy;
                        parts[i].vx *= ptypes[t].collision;
                        parts[i].vy *= ptypes[t].collision;
                    }
                    else
                    {
                        r = (rand()%2)*2-1;
                        if(ny!=y && try_move(i, x, y, x+r, ny))
                        {
                            parts[i].x += r;
                            parts[i].y = iy;
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                        else if(ny!=y && try_move(i, x, y, x-r, ny))
                        {
                            parts[i].x -= r;
                            parts[i].y = iy;
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                        else if(nx!=x && try_move(i, x, y, nx, y+r))
                        {
                            parts[i].x = ix;
                            parts[i].y += r;
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                        else if(nx!=x && try_move(i, x, y, nx, y-r))
                        {
                            parts[i].x = ix;
                            parts[i].y -= r;
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                        else if(ptypes[t].falldown>1 && parts[i].vy>fabs(parts[i].vx))
                        {
                            s = 0;
                            if(!rt || nt) //nt is if there is an something else besides the current particle type, around the particle
                                rt = 30;//slight less water lag, although it changes how it moves a lot
                            else
                                rt = 10;
                            for(j=x+r; j>=0 && j>=x-rt && j<x+rt && j<XRES; j+=r)
                            {
                                if(try_move(i, x, y, j, ny))
                                {
                                    parts[i].x += j-x;
                                    parts[i].y += ny-y;
                                    x = j;
                                    y = ny;
                                    s = 1;
                                    break;
                                }
                                if(try_move(i, x, y, j, y))
                                {
                                    parts[i].x += j-x;
                                    x = j;
                                    s = 1;
                                    break;
                                }
                                if((pmap[y][j]&255)!=t || (bmap[y/CELL][j/CELL] && bmap[y/CELL][j/CELL]!=WL_STREAM))
                                    break;
                            }
                            if(parts[i].vy>0)
                                r = 1;
                            else
                                r = -1;
                            if(s)
                                for(j=y+r; j>=0 && j<YRES && j>=y-rt && j<y+rt; j+=r)
                                {
                                    if(try_move(i, x, y, x, j))
                                    {
                                        parts[i].y += j-y;
                                        break;
                                    }
                                    if((pmap[j][x]&255)!=t || (bmap[j/CELL][x/CELL] && bmap[j/CELL][x/CELL]!=WL_STREAM))
                                    {
                                        s = 0;
                                        break;
                                    }
                                }
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                            if(!s)
                                parts[i].flags |= FLAG_STAGNANT;
                        }
                        else
                        {
                            parts[i].flags |= FLAG_STAGNANT;
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                    }
                }
                else
                {
                    parts[i].flags |= FLAG_STAGNANT;
                    if(t==PT_NEUT && 100>(rand()%1000))
                    {
                        kill_part(i);
                        continue;
                    }
                    else if(t==PT_NEUT || t==PT_PHOT) //Seems to break neutrons, sorry Skylark
                    {
                        r = pmap[ny][nx];

                        /* this should be replaced with a particle type attribute ("photwl" or something) */
                        if((r & 0xFF) == PT_PSCN) parts[i].ctype  = 0x00000000;
                        if((r & 0xFF) == PT_NSCN) parts[i].ctype  = 0x00000000;
                        if((r & 0xFF) == PT_SPRK) parts[i].ctype  = 0x00000000;
                        if((r & 0xFF) == PT_COAL) parts[i].ctype  = 0x00000000;
                        if((r & 0xFF) == PT_BCOL) parts[i].ctype  = 0x00000000;
                        if((r & 0xFF) == PT_PLEX) parts[i].ctype &= 0x1F00003E;
                        if((r & 0xFF) == PT_NITR) parts[i].ctype &= 0x0007C000;
                        if((r & 0xFF) == PT_NBLE) parts[i].ctype &= 0x3FFF8000;
                        if((r & 0xFF) == PT_LAVA) parts[i].ctype &= 0x3FF00000;
                        if((r & 0xFF) == PT_ACID) parts[i].ctype &= 0x1FE001FE;
                        if((r & 0xFF) == PT_DUST) parts[i].ctype &= 0x3FFFFFC0;
                        if((r & 0xFF) == PT_SNOW) parts[i].ctype &= 0x03FFFFFF;
                        if((r & 0xFF) == PT_GOO)  parts[i].ctype &= 0x3FFAAA00;
                        if((r & 0xFF) == PT_PLNT) parts[i].ctype &= 0x0007C000;
                        if((r & 0xFF) == PT_PLUT) parts[i].ctype &= 0x001FCE00;
                        if((r & 0xFF) == PT_URAN) parts[i].ctype &= 0x003FC000;

                        if(get_normal_interp(t, lx, ly, parts[i].vx, parts[i].vy, &nrx, &nry)) {
                            dp = nrx*parts[i].vx + nry*parts[i].vy;
                            parts[i].vx -= 2.0f*dp*nrx;
                            parts[i].vy -= 2.0f*dp*nry;
                            nx = (int)(parts[i].x + parts[i].vx + 0.5f);
                            ny = (int)(parts[i].y + parts[i].vy + 0.5f);
                            if(try_move(i, x, y, nx, ny)) {
                                parts[i].x = (float)nx;
                                parts[i].y = (float)ny;
                            } else {
                                kill_part(i);
                                continue;
                            }
                        } else {
			    if(t!=PT_NEUT)
                            	kill_part(i);
                            continue;
                        }

                        if(!parts[i].ctype) {
			    if(t!=PT_NEUT)
                            	kill_part(i);
                            continue;
                        }
                    }

                    else
                    {
                        if(nx>x+ISTP) nx=x+ISTP;
                        if(nx<x-ISTP) nx=x-ISTP;
                        if(ny>y+ISTP) ny=y+ISTP;
                        if(ny<y-ISTP) ny=y-ISTP;
                        if(try_move(i, x, y, 2*x-nx, ny))
                        {
                            parts[i].x = (float)(2*x-nx);
                            parts[i].y = (float)iy;
                            parts[i].vx *= ptypes[t].collision;
                        }
                        else if(try_move(i, x, y, nx, 2*y-ny))
                        {
                            parts[i].x = (float)ix;
                            parts[i].y = (float)(2*y-ny);
                            parts[i].vy *= ptypes[t].collision;
                        }
                        else
                        {
                            parts[i].vx *= ptypes[t].collision;
                            parts[i].vy *= ptypes[t].collision;
                        }
                    }
                }
            }
            if(nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)
            {
                kill_part(i);
                continue;
            }
        }
    if(framerender) {
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
    memset(pmap, 0, sizeof(pmap));
    r = rand()%2;
    NUM_PARTS = 0;
    for(j=0; j<NPART; j++)
    {
        i = r ? (NPART-1-j) : j;
        if(parts[i].type)
        {
            t = parts[i].type;
            x = (int)(parts[i].x+0.5f);
            y = (int)(parts[i].y+0.5f);
            if(x>=0 && y>=0 && x<XRES && y<YRES && t!=PT_PHOT) {
                if(t!=PT_NEUT || (pmap[y][x]&0xFF)!=PT_GLAS)
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
    if(cmode==CM_BLOB)
    {
        for(y=0; y<YRES/CELL; y++)
        {
            for(x=0; x<XRES/CELL; x++)
            {
                if(bmap[y][x]==WL_WALL)
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);

                        }
                if(bmap[y][x]==WL_DESTROYALL)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                        }
                if(bmap[y][x]==WL_ALLOWLIQUID)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
                                drawblob(vid, (x*CELL+i), (y*CELL+j), 0xC0, 0xC0, 0xC0);
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
                if(bmap[y][x]==WL_FAN)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0xFF);
                        }
                if(bmap[y][x]==WL_DETECT)
                {
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFF8080);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0xFF, 0x80, 0x80);
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
                if(bmap[y][x]==WL_EWALL)
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
                                    vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                                    drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                                }
                    }
                    else
                    {
                        for(j=0; j<CELL; j++)
                            for(i=0; i<CELL; i++)
                                pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                        for(j=0; j<CELL; j++)
                            for(i=0; i<CELL; i++)
                                if(!(i&j&1))
                                {
                                    vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                                    drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                                }
                    }
                }
                if(bmap[y][x]==WL_WALLELEC)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
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
                if(bmap[y][x]==WL_ALLOWALLELEC)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            //pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFFFF22);
                                drawblob(vid, (x*CELL+i), (y*CELL+j), 0xFF, 0xFF, 0x22);
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
                if(bmap[y][x]==WL_ALLOWGAS)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x579777);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x57, 0x97, 0x77);
                        }
                    }
                }
                if(bmap[y][x]==WL_ALLOWAIR)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x3C3C3C);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x3C, 0x3C, 0x3C);
                        }
                    }
                }
                if(bmap[y][x]==WL_ALLOWSOLID)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x575757);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x57, 0x57, 0x57);
                        }
                    }
                }
                if(bmap[y][x]==WL_EHOLE)
                {
                    if(emap[y][x])
                    {
                        for(j=0; j<CELL; j++)
                        {
                            for(i=(j)&1; i<CELL; i++)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
                                drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
                            }
                        }
                        for(j=0; j<CELL; j+=2)
                        {
                            for(i=(j)&1; i<CELL; i+=2)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
                            }
                        }
                    }
                    else
                    {
                        for(j=0; j<CELL; j+=2)
                        {
                            for(i=(j)&1; i<CELL; i+=2)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
                                drawblob(vid, (x*CELL+i), (y*CELL+j), 0x24, 0x24, 0x24);
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
                if(emap[y][x] && (!sys_pause||framerender))
                    emap[y][x] --;
            }
        }
    }
    else
    {
        for(y=0; y<YRES/CELL; y++)
        {
            for(x=0; x<XRES/CELL; x++)
            {
                if(bmap[y][x]==WL_WALL)
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                        }
                if(bmap[y][x]==WL_DESTROYALL)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                if(bmap[y][x]==WL_ALLOWLIQUID)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
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
                if(bmap[y][x]==WL_FAN)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
                if(bmap[y][x]==WL_DETECT)
                {
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFF8080);
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
                if(bmap[y][x]==WL_EWALL)
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
                                    vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                    }
                    else
                    {
                        for(j=0; j<CELL; j++)
                            for(i=0; i<CELL; i++)
                                pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                        for(j=0; j<CELL; j++)
                            for(i=0; i<CELL; i++)
                                if(!(i&j&1))
                                    vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                    }
                }
                if(bmap[y][x]==WL_WALLELEC)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xC0C0C0);
                            else
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
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
                if(bmap[y][x]==WL_ALLOWALLELEC)
                {
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            //pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            if(!((y*CELL+j)%2) && !((x*CELL+i)%2))
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0xFFFF22);

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
                if(bmap[y][x]==WL_ALLOWAIR)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x3C3C3C);
                        }
                    }
                }
                if(bmap[y][x]==WL_ALLOWGAS)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x579777);
                        }
                    }
                }
                if(bmap[y][x]==WL_ALLOWSOLID)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x575757);
                        }
                    }
                }
                if(bmap[y][x]==WL_EHOLE)
                {
                    if(emap[y][x])
                    {
                        for(j=0; j<CELL; j++)
                        {
                            for(i=(j)&1; i<CELL; i++)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
                            }
                        }
                        for(j=0; j<CELL; j+=2)
                        {
                            for(i=(j)&1; i<CELL; i+=2)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x000000);
                            }
                        }
                    }
                    else
                    {
                        for(j=0; j<CELL; j+=2)
                        {
                            for(i=(j)&1; i<CELL; i+=2)
                            {
                                vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x242424);
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
                if(emap[y][x] && (!sys_pause||framerender))
                    emap[y][x] --;
            }
        }
    }

    update_particles_i(vid, 0, 1);

    for(y=0; y<YRES/CELL; y++)
        for(x=0; x<XRES/CELL; x++)
            if(bmap[y][x]==WL_STREAM)
            {
                lx = x*CELL + CELL*0.5f;
                ly = y*CELL + CELL*0.5f;
                for(t=0; t<1024; t++)
                {
                    nx = (int)(lx+0.5f);
                    ny = (int)(ly+0.5f);
                    if(nx<0 || nx>=XRES || ny<0 || ny>=YRES)
                        break;
                    addpixel(vid, nx, ny, 255, 255, 255, 64);
                    i = nx/CELL;
                    j = ny/CELL;
                    lx += vx[j][i]*0.125f;
                    ly += vy[j][i]*0.125f;
                    if(bmap[j][i]==WL_STREAM && i!=x && j!=y)
                        break;
                }
                drawtext(vid, x*CELL, y*CELL-2, "\x8D", 255, 255, 255, 128);
            }

}

void clear_area(int area_x, int area_y, int area_w, int area_h)
{
    int cx = 0;
    int cy = 0;
    for(cy=0; cy<area_h; cy++)
    {
        for(cx=0; cx<area_w; cx++)
        {
            bmap[(cy+area_y)/CELL][(cx+area_x)/CELL] = 0;
            delete_part(cx+area_x, cy+area_y);
        }
    }
}

void create_box(int x1, int y1, int x2, int y2, int c)
{
    int i, j;
    if(x1>x2)
    {
        i = x2;
        x2 = x1;
        x1 = i;
    }
    if(y1>y2)
    {
        j = y2;
        y2 = y1;
        y1 = j;
    }
    for(j=y1; j<=y2; j++)
        for(i=x1; i<=x2; i++)
            create_parts(i, j, 1, 1, c);
}

int flood_parts(int x, int y, int c, int cm, int bm)
{
    int x1, x2, dy = (c<PT_NUM)?1:CELL;
    int co = c;
    if(cm==PT_INST&&co==PT_SPRK)
	    if((pmap[y][x]&0xFF)==PT_SPRK)
		    return 0;
    if(c>=UI_WALLSTART&&c<=UI_WALLSTART+UI_WALLCOUNT)
    {
        c = c-100;
    }
    if(cm==-1)
    {
	if(c==0)
        {
            cm = pmap[y][x]&0xFF;
            if(!cm)
                return 0;
	    if(REPLACE_MODE && cm!=SLALT)
		return 0;
        }
        else
            cm = 0;
    }
    if(bm==-1)
    {
        if(c==WL_ERASE)
        {
            bm = bmap[y/CELL][x/CELL];
            if(!bm)
                return 0;
            if(bm==WL_WALL)
                cm = 0xFF;
        }
        else
            bm = 0;
    }

    if(((pmap[y][x]&0xFF)!=cm || bmap[y/CELL][x/CELL]!=bm )||( (sdl_mod & (KMOD_CAPS)) && cm!=SLALT))
        return 1;

    // go left as far as possible
    x1 = x2 = x;
    while(x1>=CELL)
    {
        if((pmap[y][x1-1]&0xFF)!=cm || bmap[y/CELL][(x1-1)/CELL]!=bm)
	{
			break;
	}
        x1--;
    }
    while(x2<XRES-CELL)
    {
        if((pmap[y][x2+1]&0xFF)!=cm || bmap[y/CELL][(x2+1)/CELL]!=bm)
	{
			break;
	}
        x2++;
    }

    // fill span
    for(x=x1; x<=x2; x++)
    {
        if(!create_parts(x, y, 0, 0, co))
            return 0;
    }
    // fill children
    if(cm==PT_INST&&co==PT_SPRK)//wire crossing for INST
    {
	if(y>=CELL+dy && x1==x2 &&
		((pmap[y-1][x1-1]&0xFF)==PT_INST||(pmap[y-1][x1-1]&0xFF)==PT_SPRK) && ((pmap[y-1][x1]&0xFF)==PT_INST||(pmap[y-1][x1]&0xFF)==PT_SPRK) && ((pmap[y-1][x1+1]&0xFF)==PT_INST || (pmap[y-1][x1+1]&0xFF)==PT_SPRK) &&
		(pmap[y-2][x1-1]&0xFF)!=PT_INST && ((pmap[y-2][x1]&0xFF)==PT_INST ||(pmap[y-2][x1]&0xFF)==PT_SPRK) && (pmap[y-2][x1+1]&0xFF)!=PT_INST)
			flood_parts(x1, y-2, co, cm, bm);
	else if(y>=CELL+dy)
	    for(x=x1; x<=x2; x++)
		if((pmap[y-1][x]&0xFF)!=PT_SPRK)
		{
			if(x==x1 || x==x2 || y>=YRES-CELL-1 ||
				(pmap[y-1][x-1]&0xFF)==PT_INST || (pmap[y-1][x+1]&0xFF)==PT_INST ||
				(pmap[y+1][x-1]&0xFF)==PT_INST || ((pmap[y+1][x]&0xFF)!=PT_INST&&(pmap[y+1][x]&0xFF)!=PT_SPRK) || (pmap[y+1][x+1]&0xFF)==PT_INST)
					flood_parts(x, y-dy, co, cm, bm);

            }

	if(y<YRES-CELL-dy && x1==x2 &&
		((pmap[y+1][x1-1]&0xFF)==PT_INST||(pmap[y+1][x1-1]&0xFF)==PT_SPRK) && ((pmap[y+1][x1]&0xFF)==PT_INST||(pmap[y+1][x1]&0xFF)==PT_SPRK) && ((pmap[y+1][x1+1]&0xFF)==PT_INST || (pmap[y+1][x1+1]&0xFF)==PT_SPRK) &&
		(pmap[y+2][x1-1]&0xFF)!=PT_INST && ((pmap[y+2][x1]&0xFF)==PT_INST ||(pmap[y+2][x1]&0xFF)==PT_SPRK) && (pmap[y+2][x1+1]&0xFF)!=PT_INST)
			flood_parts(x1, y+2, co, cm, bm);
	else if(y<YRES-CELL-dy)
            for(x=x1; x<=x2; x++)
		if((pmap[y+1][x]&0xFF)!=PT_SPRK)
		{
			if(x==x1 || x==x2 || y<0 ||
				(pmap[y+1][x-1]&0xFF)==PT_INST || (pmap[y+1][x+1]&0xFF)==PT_INST ||
				(pmap[y-1][x-1]&0xFF)==PT_INST || ((pmap[y-1][x]&0xFF)!=PT_INST&&(pmap[y-1][x]&0xFF)!=PT_SPRK) || (pmap[y-1][x+1]&0xFF)==PT_INST)
					flood_parts(x, y+dy, co, cm, bm);

		}
    }
    else
    {
    if(y>=CELL+dy)
        for(x=x1; x<=x2; x++)
            if((pmap[y-dy][x]&0xFF)==cm && bmap[(y-dy)/CELL][x/CELL]==bm)
                if(!flood_parts(x, y-dy, co, cm, bm))
                    return 0;
    if(y<YRES-CELL-dy)
        for(x=x1; x<=x2; x++)
            if((pmap[y+dy][x]&0xFF)==cm && bmap[(y+dy)/CELL][x/CELL]==bm)
                if(!flood_parts(x, y+dy, co, cm, bm))
                    return 0;
    }
    if(!(cm==PT_INST&&co==PT_SPRK))
	return 1;
}

int create_parts(int x, int y, int rx, int ry, int c)
{
    int i, j, r, f = 0, u, v, oy, ox, b = 0, dw = 0, stemp = 0;//n;
    
    int wall = c - 100;
    for(r=UI_ACTUALSTART;r<=UI_ACTUALSTART+UI_WALLCOUNT;r++)
    {
	if(wall==r)
	    {
		    if(c == SPC_AIR || c == SPC_HEAT || c == SPC_COOL || c == SPC_VACUUM)
			    break;
		    if(wall == WL_ERASE)
			b = 0;
		    else
		    	b = wall;
		    dw = 1;
	    }
    }
    if(c == WL_FANHELPER)
    {
        b = WL_FANHELPER;
        dw = 1;
    }
    if(dw==1)
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
                if(ox>=0&&ox<XRES/CELL&&oy>=0&&oy<YRES/CELL)
                {
                    i = ox;
                    j = oy;
		    if(((sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))|| sdl_mod & (KMOD_CAPS) ))
		    {
			    if(bmap[j][i]==SLALT-100)
				    b = 0;
			    else
				    continue;
		    }
                    if(b==WL_FAN)
                    {
                        fvx[j][i] = 0.0f;
                        fvy[j][i] = 0.0f;
                    }
		    if(b==WL_STREAM)
		    {
				i = x + rx/2;
				j = y + rx/2;
				for(v=-1; v<2; v++)
				for(u=-1; u<2; u++)
				if(i+u>=0 && i+u<XRES/CELL &&
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
    
    if(((sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))|| sdl_mod & (KMOD_CAPS) )&& !REPLACE_MODE)
	{
        for(j=-ry; j<=ry; j++)
            for(i=-rx; i<=rx; i++)
                if((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
                    delete_part(x+i, y+j);
        return 1;
	}

    if(c == SPC_AIR || c == SPC_HEAT || c == SPC_COOL || c == SPC_VACUUM)
    {
        for(j=-ry; j<=ry; j++)
            for(i=-rx; i<=rx; i++)
                if((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
                    create_part(-1, x+i, y+j, c);
        return 1;
    }

    if(c == 0 && !REPLACE_MODE)
    {
	stemp = SLALT;
	SLALT = 0;
        for(j=-ry; j<=ry; j++)
            for(i=-rx; i<=rx; i++)
                if((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
                    delete_part(x+i, y+j);
	SLALT = stemp;
        return 1;
    }
    if(REPLACE_MODE)
    {
        for(j=-ry; j<=ry; j++)
            for(i=-rx; i<=rx; i++)
                if((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
		{
		    if((pmap[y+j][x+i]&0xFF)!=SLALT&&SLALT!=0)
			continue;
		    if((pmap[y+j][x+i]))
		    {
			delete_part(x+i, y+j);
			if(c!=0)
			create_part(-1, x+i, y+j, c);
		    }
		}
		return 1;
		
    }
    for(j=-ry; j<=ry; j++)
        for(i=-rx; i<=rx; i++)
            if((CURRENT_BRUSH==CIRCLE_BRUSH && (pow(i,2))/(pow(rx,2))+(pow(j,2))/(pow(ry,2))<=1)||(CURRENT_BRUSH==SQUARE_BRUSH&&i*j<=ry*rx))
                if(create_part(-1, x+i, y+j, c)==-1)
                    f = 1;
    return !f;
}

void create_line(int x1, int y1, int x2, int y2, int rx, int ry, int c)
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
            create_parts(y, x, rx, ry, c);
        else
            create_parts(x, y, rx, ry, c);
        e += de;
        if(e >= 0.5f)
        {
            y += sy;
            if(c==WL_EHOLE || c==WL_ALLOWGAS || c==WL_ALLOWALLELEC || c==WL_ALLOWSOLID || c==WL_ALLOWAIR || c==WL_WALL || c==WL_DESTROYALL || c==WL_ALLOWLIQUID || c==WL_FAN || c==WL_STREAM || c==WL_DETECT || c==WL_EWALL || c==WL_WALLELEC || !(rx+ry))
            {
                if(cp)
                    create_parts(y, x, rx, ry, c);
                else
                    create_parts(x, y, rx, ry, c);
            }
            e -= 1.0f;
        }
    }
}
