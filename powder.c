#include <math.h>
#include "defines.h"
#include "powder.h"
#include "air.h"
#include "misc.h"

int isplayer = 0;
float player[20]; //[0] is a command cell, [3]-[18] are legs positions, [19] is index

particle *parts;
particle *cb_parts;

unsigned char bmap[YRES/CELL][XRES/CELL];
unsigned char emap[YRES/CELL][XRES/CELL];

unsigned char cb_bmap[YRES/CELL][XRES/CELL];
unsigned char cb_emap[YRES/CELL][XRES/CELL];

int pfree;

unsigned pmap[YRES][XRES];
unsigned cb_pmap[YRES][XRES];

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

    if(parts[i].type==PT_PHOT&&((r&0xFF)==PT_GLAS||(r&0xFF)==PT_PHOT||(r&0xFF)==PT_CLNE||((r&0xFF)==PT_LCRY&&parts[r>>8].life > 5)))
    {
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

    if(parts[i].type==PT_STKM)  //Stick man's head shouldn't collide
    {
        return 1;
    }

    if(bmap[ny/CELL][nx/CELL]==12 && !emap[y/CELL][x/CELL])
    {
        return 1;
    }
    if(bmap[ny/CELL][nx/CELL]==13 && ptypes[parts[i].type].falldown!=0 && parts[i].type!=PT_FIRE)
    {
        return 0;
    }
    if((bmap[y/CELL][x/CELL]==12 && !emap[y/CELL][x/CELL]) && (bmap[ny/CELL][nx/CELL]!=12 && !emap[ny/CELL][nx/CELL]))
    {
        return 0;
    }

    if(ptypes[parts[i].type].falldown!=2 && bmap[ny/CELL][nx/CELL]==3)
        return 0;
    if((parts[i].type==PT_NEUT ||parts[i].type==PT_PHOT) && bmap[ny/CELL][nx/CELL]==7 && !emap[ny/CELL][nx/CELL])
        return 0;
    if(r && (r>>8)<NPART && ptypes[r&0xFF].falldown!=2 && bmap[y/CELL][x/CELL]==3)
        return 0;

    if(bmap[ny/CELL][nx/CELL]==9)
        return 0;

    if(ptypes[parts[i].type].falldown!=1 && bmap[ny/CELL][nx/CELL]==10)
        return 0;

    if (r && ((r&0xFF) >= PT_NUM || !can_move[parts[i].type][(r&0xFF)]))
        return 0;

    if(parts[i].type==PT_CNCT && y<ny && (pmap[y+1][x]&0xFF)==PT_CNCT)
    {
        return 0;
    }

    pmap[ny][nx] = (i<<8)|parts[i].type;
    pmap[y][x] = r;
    if(r && (r>>8)<NPART)
    {
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

#ifdef WIN32
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
                parts[pmap[y][x]>>8].temp = restrict_flt(parts[pmap[y][x]>>8].temp + 4.0f, MIN_TEMP, MAX_TEMP);
            }
            if(t==SPC_COOL&&parts[pmap[y][x]>>8].temp>MIN_TEMP)
            {
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
				(pmap[y][x]&0xFF)!=PT_INWR)
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
    if(t==PT_FIRE)
        parts[i].life = rand()%50+120;
    if(t==PT_PLSM)
        parts[i].life = rand()%150+50;
    if(t==PT_LAVA)
        parts[i].life = rand()%120+240;
    if(t==PT_NBLE)
        parts[i].life = 0;
    if(t==PT_NEUT)
    {
        float r = (rand()%128+128)/127.0f;
        float a = (rand()%360)*3.14159f/180.0f;
        parts[i].life = rand()%480+480;
        parts[i].vx = r*cosf(a);
        parts[i].vy = r*sinf(a);
    }
    if(t==PT_PHOT)
    {
        float r = (rand()%3-1)*3;
        float a = (rand()%3-1)*3;
        parts[i].life = 680;
        if(a==0.0f&&r==0.0f)
        {
            parts[i].life = 0;
            parts[i].type = PT_NONE;
            return -1;
        }
        else
        {
            parts[i].vx = a;
            parts[i].vy = r;
        }
        //} else {
        //	parts[i].life = 0;
        //	parts[i].type = PT_NONE;
        //}/
    }

    if(t!=PT_STKM)
        pmap[y][x] = t|(i<<8);
    else
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

#ifdef WIN32
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

    kill_part(i>>8);
    pmap[y][x] = 0;	// just in case
}

#ifdef WIN32
_inline int is_wire(int x, int y)
#else
inline int is_wire(int x, int y)
#endif
{
    return bmap[y][x]==6 || bmap[y][x]==7 || bmap[y][x]==3 || bmap[y][x]==8 || bmap[y][x]==11 || bmap[y][x]==12;
}

#ifdef WIN32
_inline int is_wire_off(int x, int y)
#else
inline int is_wire_off(int x, int y)
#endif
{
    return (bmap[y][x]==6 || bmap[y][x]==7 || bmap[y][x]==3 || bmap[y][x]==8 || bmap[y][x]==11 || bmap[y][x]==12) && emap[y][x]<8;
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

#ifdef WIN32
_inline int parts_avg(int ci, int ni)
#else
inline int parts_avg(int ci, int ni)
#endif
{
    int pmr = pmap[(int)((parts[ci].y + parts[ni].y)/2)][(int)((parts[ci].x + parts[ni].x)/2)];
    if((pmr>>8) < NPART && (pmr>>8) >= 0)
    {
        return parts[pmr>>8].type;
    }
    else
    {
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
    int i, j, x, y, t, nx, ny, r, a, cr,cg,cb, s, rt, fe, nt, lpv, nearp, pavg;
    float mv, dx, dy, ix, iy, lx, ly, d, pp;
    float pt = R_TEMP;
    float c_heat = 0.0f;
    int h_count = 0;
    int starti = (start*-1);
    for(i=start; i<(NPART-starti); i+=inc)
        if(parts[i].type)
        {
			//printf("parts[%d].type: %d\n", i, parts[i].type);

            lx = parts[i].x;
            ly = parts[i].y;
            t = parts[i].type;

            if(sys_pause&&!framerender)
				return;

            if(parts[i].life && t!=PT_ACID && t!=PT_WOOD && t!=PT_NBLE && t!=PT_SWCH && t!=PT_STKM)
            {
                if(!(parts[i].life==10&&parts[i].type==PT_LCRY))
                    parts[i].life--;
                if(parts[i].life<=0 && t!=PT_METL && t!=PT_WATR && t!=PT_RBDM && t!=PT_LRBD && t!=PT_SLTW && t!=PT_BRMT && t!=PT_PSCN && t!=PT_NSCN && t!=PT_NTCT && t!=PT_PTCT && t!=PT_BMTL && t!=PT_SPRK && t!=PT_LAVA && t!=PT_ETRD&&t!=PT_LCRY && t!=PT_INWR)
                {
                    kill_part(i);
                    continue;
                }
                if(parts[i].life<=0 && t==PT_SPRK)
                {
                    t = parts[i].ctype;
                    if(t!=PT_METL&&t!=PT_BMTL&&t!=PT_BRMT&&t!=PT_LRBD&&t!=PT_RBDM&&t!=PT_BTRY&&t!=PT_NBLE)
                        parts[i].temp = R_TEMP;
                    if(!t)
                        t = PT_METL;
                    parts[i].type = t;
                    parts[i].life = 4;
                    if(t == PT_WATR)
                        parts[i].life = 64;
                    if(t == PT_SLTW)
                        parts[i].life = 54;
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
                    ((bmap[y/CELL][x/CELL]==1 ||
                      bmap[y/CELL][x/CELL]==8 ||
                      bmap[y/CELL][x/CELL]==9 ||
                      (bmap[y/CELL][x/CELL]==2) ||
                      (bmap[y/CELL][x/CELL]==3 && ptypes[t].falldown!=2) ||
                      (bmap[y/CELL][x/CELL]==10 && ptypes[t].falldown!=1) ||
                      (bmap[y/CELL][x/CELL]==6 && (t==PT_METL || t==PT_SPRK)) ||
                      (bmap[y/CELL][x/CELL]==7 && !emap[y/CELL][x/CELL])) && (t!=PT_STKM)))
            {
                kill_part(i);
                continue;
            }

            vx[y/CELL][x/CELL] *= ptypes[t].airloss;
            vy[y/CELL][x/CELL] *= ptypes[t].airloss;
            vx[y/CELL][x/CELL] += ptypes[t].airdrag*parts[i].vx;
            vy[y/CELL][x/CELL] += ptypes[t].airdrag*parts[i].vy;
            if(t==PT_GAS||t==PT_NBLE)
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

            if(t==PT_GOO && !parts[i].life)
            {
                if(pv[y/CELL][x/CELL]>1.0f)
                {
                    parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL];
                    parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL];
                    parts[i].life = rand()%80+300;
                }
            }
            else
            {
				if(t==PT_PLAS && pv[y/CELL][x/CELL]>25.0f)
				{
					parts[i].vx += ptypes[t].advection*vx[y/CELL][x/CELL];
					parts[i].vy += ptypes[t].advection*vy[y/CELL][x/CELL];
				} else {
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
#ifdef WIN32
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
                    if(nx<0 || ny<0 || nx>=XRES || ny>=YRES || pmap[ny][nx] || (bmap[ny/CELL][nx/CELL] && bmap[ny/CELL][nx/CELL]!=5))
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
                            (!bmap[(y+ny)/CELL][(x+nx)/CELL] || bmap[(y+ny)/CELL][(x+nx)/CELL]==5))
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
                if(t==PT_DESL && pv[y/CELL][x/CELL]<-6.0f)
                    t = parts[i].type = PT_GAS;
                if(t==PT_GAS && pv[y/CELL][x/CELL]>6.0f)
                    t = parts[i].type = PT_DESL;
                if(t==PT_DESL && pv[y/CELL][x/CELL]>12.0f)
                    t = parts[i].type = PT_FIRE;
            }
            if(t==PT_DESL && pv[y/CELL][x/CELL]<-20.0f)
                t = parts[i].type = PT_GAS;
            if(t==PT_DESL && pv[y/CELL][x/CELL]>50.0f)      // Only way I know to make it
                t = parts[i].type = PT_FIRE;                // combust under pressure.
            if(t==PT_GAS && pv[y/CELL][x/CELL]>20.0f)
                t = parts[i].type = PT_DESL;
            if(t==PT_BMTL && pv[y/CELL][x/CELL]>2.5f)
                t = parts[i].type = PT_BRMT;
            //if(t==PT_GLAS && pv[y/CELL][x/CELL]>4.0f)
            //	t = parts[i].type = PT_BGLA;
            if(t==PT_GLAS)
            {
                parts[i].pavg[0] = parts[i].pavg[1];
                parts[i].pavg[1] = pv[y/CELL][x/CELL];
                if(parts[i].pavg[1]-parts[i].pavg[0] > 0.05f || parts[i].pavg[1]-parts[i].pavg[0] < -0.05f)
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

            if(t==PT_SPRK&&parts[i].ctype==PT_ETRD&&parts[i].life==1)
            {
                nearp = nearest_part(i, PT_ETRD);
                if(nearp!=-1)
                {
                    create_line((int)parts[i].x, (int)parts[i].y, (int)parts[nearp].x, (int)parts[nearp].y, 0, PT_PLSM);
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
                if(ptypes[t].hconduct>(rand()%250))
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
                                if(parts[r>>8].type!=PT_NONE&&parts[i].type!=PT_NONE&&ptypes[parts[r>>8].type].hconduct>0)
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
                                if(parts[r>>8].type!=PT_NONE&&parts[i].type!=PT_NONE&&ptypes[parts[r>>8].type].hconduct>0)
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
                            pv[y/CELL][x/CELL] += 0.50f;
                            if(t==PT_FIRE)
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
                    }
                    pt = parts[i].temp = restrict_flt(parts[i].temp, MIN_TEMP, MAX_TEMP);
                }
            }
            if(t==PT_PTCT&&parts[i].temp>24.0f)
            {
                pt = parts[i].temp -= 2.5f;
            }
            if(t==PT_NTCT&&parts[i].temp>24.0f)
            {
                pt = parts[i].temp -= 2.5f;
            }

            if(t==PT_WATR || t==PT_ETRD || t==PT_SLTW || t==PT_METL || t==PT_RBDM || t==PT_LRBD || t==PT_BRMT || t==PT_PSCN || t==PT_NSCN || t==PT_NTCT || t==PT_PTCT || t==PT_BMTL || t==PT_SPRK|| t == PT_NBLE || t==PT_INWR)
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
                    if(t==PT_WATR || t==PT_ETRD || t==PT_SLTW || t==PT_METL || t==PT_RBDM || t==PT_LRBD || t==PT_NSCN || t==PT_NTCT || t==PT_PTCT || t==PT_PSCN || t==PT_BRMT || t==PT_BMTL||t==PT_NBLE || t==PT_INWR)
                    {
                        if(emap[ny][nx]==12 && !parts[i].life)
                        {
                            parts[i].type = PT_SPRK;
                            parts[i].life = 4;
                            parts[i].ctype = t;
                            t = PT_SPRK;
                        }
                    }
                    else if(bmap[ny][nx]==6 || bmap[ny][nx]==7 || bmap[ny][nx]==3 || bmap[ny][nx]==8 || bmap[ny][nx]==11 || bmap[ny][nx]==12)
                        set_emap(nx, ny);
                }
            }

            nx = x/CELL;
            ny = y/CELL;
            if(bmap[ny][nx]==6 && emap[ny][nx]<8)
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
                            if(((r&0xFF)==PT_METL || (r&0xFF)==PT_ETRD || (r&0xFF)==PT_PSCN || (r&0xFF)==PT_NSCN || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_PTCT || (r&0xFF)==PT_BMTL || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD || (r&0xFF)==PT_BRMT||(r&0xFF)==PT_NBLE) || (r&0xFF)==PT_INWR && parts[r>>8].ctype!=PT_SPRK )
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
                            if((r&0xFF)==PT_SPRK && parts[r>>8].ctype==PT_METL)
                            {
                                parts[i].temp = 200.0f;
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
                            }
                            else if((r&0xFF)==PT_LAVA && 1>(rand()%250))
                            {
                                parts[i].life = 4;
                                t = parts[i].type = PT_FIRE;
                            }
                            //if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
                            //t = parts[i].type = PT_WATR;
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
								if(1>(rand()%500)){
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
                                parts[r>>8].type = PT_SLTW;
                            }
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
                                else if(((r&0xFF)!=PT_CLNE && ptypes[parts[r>>8].type].hardness>(rand()%1000))&&parts[i].life>=50)
                                {
                                    parts[i].life--;
                                    parts[r>>8].type = PT_NONE;
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
                                    create_part(r>>8, x+nx, y+ny, rand()%2 ? PT_LAVA : PT_URAN);
                                }
                                else
                                {
                                    create_part(r>>8, x+nx, y+ny, PT_NEUT);
                                    parts[r>>8].vx = 0.25f*parts[r>>8].vx + parts[i].vx;
                                    parts[r>>8].vy = 0.25f*parts[r>>8].vy + parts[i].vy;
                                }
                                pv[y/CELL][x/CELL] += 10.00f * CFDS; //Used to be 2, some people said nukes weren't powerful enough
                                fe ++;
                            }
                            if((r&0xFF)==PT_GUNP && 15>(rand()%1000))
                                parts[r>>8].type = PT_DUST;
                            if((r&0xFF)==PT_DYST && 15>(rand()%1000))
                                parts[r>>8].type = PT_YEST;
							if((r&0xFF)==PT_YEST){
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
                            if((r&0xFF)==PT_OIL && 5>(rand()%1000))
                                parts[r>>8].type = PT_PLAS;
                            if((r&0xFF)==PT_PLNT && 5>(rand()%100))
                                parts[r>>8].type = PT_WOOD;
                            if((r&0xFF)==PT_PLAS && 5>(rand()%1000))
                                parts[r>>8].type = PT_OIL;
                            if((r&0xFF)==PT_DESL && 15>(rand()%1000))
                                parts[r>>8].type = PT_GAS;
                            if((r&0xFF)==PT_COAL && 5>(rand()%100))
                                parts[r>>8].type = PT_WOOD;
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
                            if(parts_avg(i,r>>8) != PT_INSL)
                            {
                                if((rt==PT_METL||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN||rt==PT_NBLE)&&parts[r>>8].life==0 && abs(nx)+abs(ny) < 4)
                                {
                                    parts[r>>8].life = 4;
                                    parts[r>>8].ctype = rt;
                                    parts[r>>8].type = PT_SPRK;
                                }
                            }
                        }
            }else if(t==PT_SWCH)
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
							if(parts[r>>8].type == PT_SWCH&&parts_avg(i,r>>8)!=PT_INSL)
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
				if((parts[i].life>0&&parts[i].life<10)|| parts[i].life == 11)
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
                            if(bmap[(y+ny)/CELL][(x+nx)/CELL] && bmap[(y+ny)/CELL][(x+nx)/CELL]!=5)
                                continue;
                            rt = parts[r>>8].type;
                            if((a || ptypes[rt].explosive) && ((rt!=PT_RBDM && rt!=PT_LRBD && rt!=PT_INSL && rt!=PT_SWCH) || t!=PT_SPRK) &&
                                    (t!=PT_LAVA || parts[i].life>0 || (rt!=PT_STNE && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_METL && rt!=PT_ETRD && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SWCH && rt!=PT_INWR)) &&
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
                                if(t!=PT_SPRK && ptypes[rt].meltable  && ((rt!=PT_RBDM && rt!=PT_LRBD) || t!=PT_SPRK) && ((t!=PT_FIRE&&t!=PT_PLSM) || (rt!=PT_METL && rt!=PT_ETRD && rt!=PT_PSCN && rt!=PT_NSCN && rt!=PT_NTCT && rt!=PT_PTCT && rt!=PT_BMTL && rt!=PT_BRMT && rt!=PT_SALT && rt!=PT_INWR)) &&
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
                            pavg = parts_avg(i, r>>8);
                            if(rt==PT_SWCH && t==PT_SPRK)
                            {
                                pavg = parts_avg(r>>8, i);
                                if(parts[i].ctype == PT_PSCN&&pavg != PT_INSL)
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
                            pavg = parts_avg(i, r>>8);
                            if(pavg != PT_INSL)
                            {
                                if(t==PT_SPRK && (rt==PT_METL||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN||rt==PT_NBLE) && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if(!(rt==PT_PSCN&&parts[i].ctype==PT_NSCN)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN&&parts[i].temp>=100.0f)&&parts[i].ctype==PT_NTCT)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN&&parts[i].temp<=100.0f)&&parts[i].ctype==PT_PTCT)&&!(rt!=PT_PSCN&&!(rt==PT_NSCN)&&parts[i].ctype==PT_INWR) && pavg != PT_INSL &&!(parts[i].ctype==PT_SWCH&&(rt==PT_PSCN||rt==PT_NSCN)) )
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                        if(parts[r>>8].temp+10.0f<400.0f&&!legacy_enable&&!(rt==PT_LRBD||rt==PT_RBDM||rt==PT_NTCT||rt==PT_PTCT||rt==PT_INWR))
                                            parts[r>>8].temp = parts[r>>8].temp+10.0f;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_NTCT && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if((parts[i].ctype==PT_NSCN||parts[i].ctype==PT_NTCT||(parts[i].ctype==PT_PSCN&&parts[r>>8].temp>100.0f))&&pavg != PT_INSL)
                                    {
                                        parts[r>>8].type = PT_SPRK;
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                    }
                                }
                                if(t==PT_SPRK && rt==PT_PTCT && parts[r>>8].life==0 &&
                                        (parts[i].life<3 || ((r>>8)<i && parts[i].life<4)) && abs(nx)+abs(ny)<4)
                                {
                                    if((parts[i].ctype==PT_NSCN||parts[i].ctype==PT_PTCT||(parts[i].ctype==PT_PSCN&&parts[r>>8].temp<100.0f))&&pavg != PT_INSL)
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
                                if(t==PT_SPRK&&parts[i].ctype==PT_SWCH&&parts[i].life<=1)
                                {
                                    parts[i].type = PT_SWCH;
                                    parts[i].life = 11;
                                }
                            }
                        }
killed:
                if(parts[i].type == PT_NONE)
                    continue;
            }
            if(t==PT_STKM)
            {
                //Tempirature handling
                if(parts[i].temp<-30)
                    parts[i].life -= 0.2;
                if((parts[i].temp<36.6f) && (parts[i].temp>=-30))
                    parts[i].temp += 1;

                //Death
                if(parts[i].life<0 && (death == 1))  //If his HP is less that 0 or there is very big wind...
                {
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

                //Verlet integration
                pp = 2*player[3]-player[5];
                player[5] = player[3];
                player[3] = pp;
                pp = 2*player[4]-player[6];
                player[6] = player[4];
                player[4] = pp;

                pp = 2*player[7]-player[9];
                player[9] = player[7];
                player[7] = pp;
                pp = 2*player[8]-player[10]+1;
                player[10] = player[8];
                player[8] = pp;

                pp = 2*player[11]-player[13];
                player[13] = player[11];
                player[11] = pp;
                pp = 2*player[12]-player[14];
                player[14] = player[12];
                player[12] = pp;

                pp = 2*player[15]-player[17];
                player[17] = player[15];
                player[15] = pp;
                pp = 2*player[16]-player[18]+1;
                player[18] = player[16];
                player[16] = pp;

                //Go left
                if (((int)(player[0])&0x01) == 0x01)
                {
                    if (pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_LIQUID
                            && (pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF) != PT_LNTG)
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[9] += 3;
                            player[10] += 2;
                            player[5] += 2;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[17] += 3;
                            player[18] += 2;
                            player[13] +=2;
                        }
                    }
                    else
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])  //It should move another way in liquids
                        {
                            player[9] += 1;
                            player[10] += 1;
                            player[5] += 1;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[17] += 1;
                            player[18] += 1;
                            player[13] +=1;
                        }
                    }
                }

                //Go right
                if (((int)(player[0])&0x02) == 0x02)
                {
                    if (pstates[pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF].state != ST_LIQUID
                            && (pmap[(int)(parts[i].y+10)][(int)(parts[i].x)]&0xFF) != PT_LNTG)
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[9] -= 3;
                            player[10] += 2;
                            player[5] -= 2;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[17] -= 3;
                            player[18] += 2;
                            player[13] -= 2;
                        }
                    }
                    else
                    {
                        if (pmap[(int)(player[8]-1)][(int)(player[7])])
                        {
                            player[9] -= 1;
                            player[10] += 1;
                            player[5] -= 1;
                        }

                        if (pmap[(int)(player[16]-1)][(int)(player[15])])
                        {
                            player[17] -= 1;
                            player[18] += 1;
                            player[13] -= 1;
                        }

                    }
                }

                //Charge detector wall if foot inside
                if(bmap[(int)(player[8]+0.5)/CELL][(int)(player[7]+0.5)/CELL]==6)
                    set_emap((int)player[7]/CELL, (int)player[8]/CELL);
                if(bmap[(int)(player[16]+0.5)/CELL][(int)(player[15]+0.5)/CELL]==6)
                    set_emap((int)(player[15]+0.5)/CELL, (int)(player[16]+0.5)/CELL);

                //Searching for particles near head
                for(nx = -2; nx <= 2; nx++)
                    for(ny = 0; ny>=-2; ny--)
                    {
                        if(!pmap[ny+y][nx+x] || (pmap[ny+y][nx+x]>>8)>=NPART)
                            continue;
                        if((pstates[pmap[ny+y][nx+x]&0xFF].state != ST_SOLID && (pmap[ny+y][nx+x]&0xFF)!=PT_STKM
                                && (pmap[ny+y][nx+x]&0xFF)!=PT_WHOL && (pmap[ny+y][nx+x]&0xFF)!=PT_BHOL)
                                || (pmap[ny+y][nx+x]&0xFF) == PT_LNTG)
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
                            create_part(-1, nx, ny, player[2]);
                            r = pmap[ny][nx];
                            if( ((r>>8) < NPART) && (r>>8)>=0 && player[2]!=PT_PHOT)
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

                //Jump
                if (((int)(player[0])&0x04) == 0x04)
                {
                    if (pmap[(int)(player[8]-0.5)][(int)(player[7])] || pmap[(int)(player[16]-0.5)][(int)(player[15])])
                    {
                        parts[i].vy = -5;
                        player[10] += 1;
                        player[18] += 1;
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
                    if(pmap[(int)(player[16]-2)][(int)(player[15]+nx)])
                        player[15] -= nx;

                    if(pmap[(int)(player[8]-2)][(int)(player[7]+nx)])
                        player[7] -= nx;
                }

                //Collision checks
                for(ny = -2-(int)parts[i].vy; ny<=0; ny++)
                {
                    r = pmap[(int)(player[8]+ny)][(int)(player[7]+0.5)];  //This is to make coding more pleasant :-)

                    //For left leg
                    if (r && (r&0xFF)!=PT_STKM)
                    {
                        if(pstates[r&0xFF].state == ST_LIQUID || pstates[r&0xFF].state == ST_GAS || (r&0xFF)==PT_LNTG)  //Liquid checks
                        {
                            if(parts[i].y<(player[8]-10))
                                parts[i].vy = 1;
                            else
                                parts[i].vy = 0;
                            if(abs(parts[i].vx)>1)
                                parts[i].vx *= 0.5;
                        }
                        else
                        {
                            player[8] += ny-1;
                            parts[i].vy -= 0.5*parts[i].vy;
                        }
                        player[9] = player[7];
                    }

                    r = pmap[(int)(player[16]+ny)][(int)(player[15]+0.5)];

                    //For right leg
                    if (r && (r&0xFF)!=PT_STKM)
                    {
                        if(pstates[r&0xFF].state == ST_LIQUID || pstates[r&0xFF].state == ST_GAS || (r&0xFF)==PT_LNTG)
                        {
                            if(parts[i].y<(player[16]-10))
                                parts[i].vy = 1;
                            else
                                parts[i].vy = 0;
                            if(abs(parts[i].vx)>1)
                                parts[i].vx *= 0.5;
                        }
                        else
                        {
                            player[16] += ny-1;
                            parts[i].vy -= 0.5*parts[i].vy;
                        }
                        player[17] = player[15];
                    }

                    //If it falls too fast
                    if (parts[i].vy>=30)
                    {
                        parts[i].y -= 10+ny;
                        parts[i].vy = -10;
                    }

                }

                //Keeping legs distance
                if (pow((player[7] - player[15]), 2)<16 && pow((player[8]-player[16]), 2)<1)
                {
                    player[7] += 0.2;
                    player[15] -= 0.2;
                }

                if (pow((player[3] - player[11]), 2)<16 && pow((player[4]-player[12]), 2)<1)
                {
                    player[3] += 0.2;
                    player[11] -= 0.2;
                }

                //If legs touch something
                r = pmap[(int)(player[8]+0.5)][(int)(player[7]+0.5)];
                if((r&0xFF)==PT_SPRK && r && (r>>8)<NPART)  //If on charge
                {
                    parts[i].life -= (int)(rand()/1000)+38;
                }

                if (r>0 && (r>>8)<NPART)  //If hot or cold
                {
                    if(parts[r>>8].temp>=50 || parts[r>>8].temp<=-30)
                    {
                        parts[i].life -= 2;
                        player[16] -= 1;
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
                    if(parts[r>>8].temp>=50 || parts[r>>8].temp<=-30)
                    {
                        parts[i].life -= 2;
                        player[8] -= 1;
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
                                    (pmap[y+ny][x+nx]&0xFF)!=PT_STKM &&
                                    (pmap[y+ny][x+nx]&0xFF)!=0xFF)
                                parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
                }
                else
                    create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
            }
            if(t==PT_YEST)
            {
				if(parts[i].temp>30&&parts[i].temp<44){
					create_part(-1, x+rand()%3-1, y+rand()%3-1, PT_YEST);
				}
            }
            if(t==PT_PLSM&&parts[i].ctype == PT_NBLE&&parts[i].life <=1)
            {
                parts[i].type = PT_NBLE;
                parts[i].life = 0;
            }
            if (t==PT_FIRE && parts[i].life <=1)
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
                            if(!rt || nt)
                                rt = 50;
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
                                if((pmap[y][j]&255)!=t || (bmap[y/CELL][j/CELL] && bmap[y/CELL][j/CELL]!=5))
                                    break;
                            }
                            if(parts[i].vy>0)
                                r = 1;
                            else
                                r = -1;
                            if(s)
                                for(j=y+r; j>=0 && j<YRES && j>=y-rt && j<x+rt; j+=r)
                                {
                                    if(try_move(i, x, y, x, j))
                                    {
                                        parts[i].y += j-y;
                                        break;
                                    }
                                    if((pmap[j][x]&255)!=t || (bmap[j/CELL][x/CELL] && bmap[j/CELL][x/CELL]!=5))
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
                    if(nx>x+ISTP) nx=x+ISTP;
                    if(nx<x-ISTP) nx=x-ISTP;
                    if(ny>y+ISTP) ny=y+ISTP;
                    if(ny<y-ISTP) ny=y-ISTP;
                    if(t==PT_NEUT && 100>(rand()%1000))
                    {
                        kill_part(i);
                        continue;
                    }
                    else if(try_move(i, x, y, 2*x-nx, ny))
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
			if(nx<CELL || nx>=XRES-CELL || ny<CELL || ny>=YRES-CELL)
			{
                kill_part(i);
                continue;
            }
        }
	if(framerender){
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
    for(j=0; j<NPART; j++)
    {
        i = r ? (NPART-1-j) : j;
        if(parts[i].type)
        {
            t = parts[i].type;
            x = (int)(parts[i].x+0.5f);
            y = (int)(parts[i].y+0.5f);
            if(x>=0 && y>=0 && x<XRES && y<YRES)
                pmap[y][x] = t|(i<<8);
        }
        else
        {
            parts[i].life = l;
            l = i;
        }
    }
    pfree=l;
    if(cmode==4)
    {
        for(y=0; y<YRES/CELL; y++)
        {
            for(x=0; x<XRES/CELL; x++)
            {
                if(bmap[y][x]==1)
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);

                        }
                if(bmap[y][x]==2)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0x80);
                        }
                if(bmap[y][x]==3)
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
                if(bmap[y][x]==4)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
                            drawblob(vid, (x*CELL+i), (y*CELL+j), 0x80, 0x80, 0xFF);
                        }
                if(bmap[y][x]==6)
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
                if(bmap[y][x]==8)
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
                if(bmap[y][x]==11)
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
                if(bmap[y][x]==13)
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
                if(bmap[y][x]==9)
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
                if(bmap[y][x]==10)
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
                if(bmap[y][x]==12)
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
                if(bmap[y][x]==1)
                    for(j=0; j<CELL; j++)
                        for(i=0; i<CELL; i++)
                        {
                            pmap[y*CELL+j][x*CELL+i] = 0x7FFFFFFF;
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                        }
                if(bmap[y][x]==2)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x808080);
                if(bmap[y][x]==3)
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
                if(bmap[y][x]==4)
                    for(j=0; j<CELL; j+=2)
                        for(i=(j>>1)&1; i<CELL; i+=2)
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x8080FF);
                if(bmap[y][x]==6)
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
                if(bmap[y][x]==8)
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
                if(bmap[y][x]==11)
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
                if(bmap[y][x]==9)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x3C3C3C);
                        }
                    }
                }
                if(bmap[y][x]==13)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x579777);
                        }
                    }
                }
                if(bmap[y][x]==10)
                {
                    for(j=0; j<CELL; j+=2)
                    {
                        for(i=(j>>1)&1; i<CELL; i+=2)
                        {
                            vid[(y*CELL+j)*(XRES+BARSIZE)+(x*CELL+i)] = PIXPACK(0x575757);
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
            if(bmap[y][x]==5)
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
                    if(bmap[j][i]==5 && i!=x && j!=y)
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
            create_parts(i, j, 1, c);
}

int flood_parts(int x, int y, int c, int cm, int bm)
{
    int x1, x2, dy = (c<PT_NUM)?1:CELL;
    int co = c;
    if(c>=122&&c<=122+UI_WALLCOUNT)
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
        }
        else
            cm = 0;
    }
    if(bm==-1)
    {
        if(c==30)
        {
            bm = bmap[y/CELL][x/CELL];
            if(!bm)
                return 0;
            if(bm==1)
                cm = 0xFF;
        }
        else
            bm = 0;
    }

    if((pmap[y][x]&0xFF)!=cm || bmap[y/CELL][x/CELL]!=bm)
        return 1;

    // go left as far as possible
    x1 = x2 = x;
    while(x1>=CELL)
    {
        if((pmap[y][x1-1]&0xFF)!=cm || bmap[y/CELL][(x1-1)/CELL]!=bm)
            break;
        x1--;
    }
    while(x2<XRES-CELL)
    {
        if((pmap[y][x2+1]&0xFF)!=cm || bmap[y/CELL][(x2+1)/CELL]!=bm)
            break;
        x2++;
    }

    // fill span
    for(x=x1; x<=x2; x++)
        if(!create_parts(x, y, 0, co))
            return 0;

    // fill children
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
    return 1;
}

int create_parts(int x, int y, int r, int c)
{
    int i, j, f = 0, u, v, oy, ox, b = 0, dw = 0; //n;

    if(c == 125)
    {
        i = x / CELL;
        j = y / CELL;
        for(v=-1; v<2; v++)
            for(u=-1; u<2; u++)
                if(i+u>=0 && i+u<XRES/CELL &&
                        j+v>=0 && j+v<YRES/CELL &&
                        bmap[j+v][i+u] == 5)
                    return 1;
        bmap[j][i] = 5;
        return 1;
    }
    //LOLOLOLOLLOLOLOLO
    if(c == 127)
    {
        b = 4;
        dw = 1;
    }
    if(c == 122)
    {
        b = 8;
        dw = 1;
    }
    if(c == 123)
    {
        b = 7;
        dw = 1;
    }
    if(c == 124)
    {
        b = 6;
        dw = 1;
    }
    if(c == 128)
    {
        b = 3;
        dw = 1;
    }
    if(c == 129)
    {
        b = 2;
        dw = 1;
    }
    if(c == 130)
    {
        b = 0;
        dw = 1;
    }
    if(c == 131)
    {
        b = 1;
        dw = 1;
    }
    if(c == 132)
    {
        b = 9;
        dw = 1;
    }
    if(c == 133)
    {
        b = 10;
        dw = 1;
    }
    if(c == 134)
    {
        b = 11;
        dw = 1;
    }
    if(c == 135)
    {
        b = 12;
        dw = 1;
    }
    if(c == 140)
    {
        b = 13;
        dw = 1;
    }
    if(c == 255)
    {
        b = 255;
        dw = 1;
    }
    if(dw==1)
    {
        r = r/CELL;
        x = x/CELL;
        y = y/CELL;
        x -= r/2;
        y -= r/2;
        for (ox=x; ox<=x+r; ox++)
        {
            for (oy=y; oy<=y+r; oy++)
            {
                if(ox>=0&&ox<XRES/CELL&&oy>=0&&oy<YRES/CELL)
                {
                    i = ox;
                    j = oy;
                    if(b==4)
                    {
                        fvx[j][i] = 0.0f;
                        fvy[j][i] = 0.0f;
                    }
                    bmap[j][i] = b;
                }
            }
        }
        return 1;
    }
    if(c == SPC_AIR || c == SPC_HEAT || c == SPC_COOL || c == SPC_VACUUM)
    {
        for(j=-r; j<=r; j++)
            for(i=-r; i<=r; i++)
                if(i*i+j*j<=r*r)
                    create_part(-1, x+i, y+j, c);
        return 1;
    }

    if(c == 0)
    {
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
            create_parts(y, x, r, c);
        else
            create_parts(x, y, r, c);
        e += de;
        if(e >= 0.5f)
        {
            y += sy;
            if(c==135 || c==140 || c==134 || c==133 || c==132 || c==131 || c==129 || c==128 || c==127 || c==125 || c==124 || c==123 || c==122 || !r)
            {
                if(cp)
                    create_parts(y, x, r, c);
                else
                    create_parts(x, y, r, c);
            }
            e -= 1.0f;
        }
    }
}