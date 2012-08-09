/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <element.h>

#define PFLAG_NORMALSPEED 0x00010000
#define PFLAG_REVERSE 0x00020000
#define PFLAG_PAUSE 0x00040000

signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

void pushParticle(int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q, notctype=(((parts[i].ctype)%3)+2);
	if ((parts[i].tmp&0xFF) == 0 || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
		return;
	x = (int)(parts[i].x+0.5f);
	y = (int)(parts[i].y+0.5f);
	if( !(parts[i].tmp&0x200) )
	{ 
		//normal random push
		rndstore = rand();
		// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
		// so can go 5 cycles without regenerating rndstore
		for (q=0; q<3; q++)//try to push twice
		{
			rnd = rndstore&7;
			rndstore = rndstore>>3;
			rx = pos_1_rx[rnd];
			ry = pos_1_ry[rnd];
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				else if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[r>>8].ctype!=notctype && (parts[r>>8].tmp&0xFF)==0)
				{
					parts[r>>8].tmp = (parts[r>>8].tmp&~0xFF) | (parts[i].tmp&0xFF);
					parts[r>>8].temp = parts[i].temp;
					parts[r>>8].tmp2 = parts[i].tmp2;
					parts[r>>8].pavg[0] = parts[i].pavg[0];
					parts[r>>8].pavg[1] = parts[i].pavg[1];
					if (r>>8 > original)
						parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
					parts[i].tmp &= ~0xFF;
					count++;
					pushParticle(r>>8,count,original);
				}
				else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
				{
					int nnx;
					for (nnx=0; nnx<80; nnx++)
						if (!portalp[parts[r>>8].tmp][count][nnx].type)
						{
							portalp[parts[r>>8].tmp][count][nnx] = parts[i];
							portalp[parts[r>>8].tmp][count][nnx].type = (parts[i].tmp&0xFF);
							portalp[parts[r>>8].tmp][count][nnx].tmp = parts[i].pavg[0];
							portalp[parts[r>>8].tmp][count][nnx].life = parts[i].tmp2;
							portalp[parts[r>>8].tmp][count][nnx].ctype = parts[i].pavg[1];
							portalp[parts[r>>8].tmp][count][nnx].pavg[0] = 0;
							portalp[parts[r>>8].tmp][count][nnx].pavg[1] = 0;
							portalp[parts[r>>8].tmp][count][nnx].tmp2 = 0;
							parts[i].tmp &= ~0xFF;
							count++;
							break;
						}
				}
			}
		}
	}
	else //predefined 1 pixel thick pipe movement
	{
		int coords = 7 - ((parts[i].tmp>>10)&7);
		r = pmap[y+ pos_1_ry[coords]][x+ pos_1_rx[coords]];
		if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[r>>8].ctype!=notctype && (parts[r>>8].tmp&0xFF)==0)
		{
			parts[r>>8].tmp = (parts[r>>8].tmp&~0xFF) | (parts[i].tmp&0xFF);
			parts[r>>8].temp = parts[i].temp;
			parts[r>>8].tmp2 = parts[i].tmp2;
			parts[r>>8].pavg[0] = parts[i].pavg[0];
			parts[r>>8].pavg[1] = parts[i].pavg[1];
			if (r>>8 > original)
				parts[r>>8].flags |= PFLAG_NORMALSPEED;//skip particle push, normalizes speed
			parts[i].tmp &= ~0xFF;
			count++;
			pushParticle(r>>8,count,original);
		}
		else if ((r&0xFF) == PT_PRTI) //Pass particles into PRTI for a pipe speed increase
		{
			int nnx;
			for (nnx=0; nnx<80; nnx++)
				if (!portalp[parts[r>>8].tmp][count][nnx].type)
				{
					portalp[parts[r>>8].tmp][count][nnx] = parts[i];
					portalp[parts[r>>8].tmp][count][nnx].type = (parts[i].tmp&0xFF);
					portalp[parts[r>>8].tmp][count][nnx].tmp = parts[i].pavg[0];
					portalp[parts[r>>8].tmp][count][nnx].life = parts[i].tmp2;
					portalp[parts[r>>8].tmp][count][nnx].ctype = parts[i].pavg[1];
					portalp[parts[r>>8].tmp][count][nnx].pavg[0] = 0;
					portalp[parts[r>>8].tmp][count][nnx].pavg[1] = 0;
					portalp[parts[r>>8].tmp][count][nnx].tmp2 = 0;
					parts[i].tmp &= ~0xFF;
					count++;
					break;
				}
		}
		else if ((r&0xFF) == PT_NONE) //Move particles out of pipe automatically, much faster at ends
		{
			rx = pos_1_rx[coords];
			ry = pos_1_ry[coords];
			np = create_part(-1,x+rx,y+ry,parts[i].tmp&0xFF);
			if (np!=-1)
			{
				parts[np].temp = parts[i].temp;
				parts[np].life = parts[i].tmp2;
				parts[np].tmp = parts[i].pavg[0];
				parts[np].ctype = parts[i].pavg[1];
				parts[i].tmp &= ~0xFF;
			}
		}
	}
	return;
}

int reverse_changed(UPDATE_FUNC_ARGS)
{
	int change = 0;
	if ((parts[i].flags & PFLAG_REVERSE) && !(parts[i].tmp & 0x10000))
	{
		parts[i].tmp |= 0x10000;
		change = 1;
	}
	else if (!(parts[i].flags & PFLAG_REVERSE) && (parts[i].tmp & 0x10000))
	{
		parts[i].tmp &= ~0x10000;
		change = 1;
	}
	if (change)
	{
		if (parts[i].ctype == 2) //Switch colors so it goes in reverse
			parts[i].ctype = 4;
		else if (parts[i].ctype == 4)
			parts[i].ctype = 2;
		if ((parts[i].tmp & 0x100) && (parts[i].tmp & 0xFC00)) //Switch one pixel pipe direction
		{
			int coords = (parts[i].tmp>>13)&7;
			int coords2 = (parts[i].tmp>>10)&7;
			parts[i].tmp &= ~0xFC00;
			parts[i].tmp |= coords<<10;
			parts[i].tmp |= coords2<<13;
			if (coords)
				parts[i].tmp |= 0x200;
			else
				parts[i].tmp &= ~0x200;
		}
	}
	return change;
}

int pause_changed(UPDATE_FUNC_ARGS)
{
	int change = 0;
	if ((parts[i].flags & PFLAG_PAUSE) && !(parts[i].tmp & 0x20000))
	{
		parts[i].tmp |= 0x20000;
		change = 1;
	}
	else if (!(parts[i].flags & PFLAG_PAUSE) && (parts[i].tmp & 0x20000))
	{
		parts[i].tmp &= ~0x20000;
		change = 1;
	}
	if (change)
	{
		int rx, ry, r;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
			{
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r&0xFF) == PT_BRCK)
					{
						if (parts[i].flags & PFLAG_PAUSE)
							parts[r>>8].tmp = 0;
						else
							parts[r>>8].tmp = 1; //make surrounding BRCK glow
					}
				}
			}
	}
	return change;
}

int update_PIPE(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	int rnd, rndstore;
	if (parts[i].type == PT_PPIP)
	{
		if ((reverse_changed(UPDATE_FUNC_SUBCALL_ARGS) || pause_changed(UPDATE_FUNC_SUBCALL_ARGS)) && (parts[i].tmp & 0x100))
		{
			for (rx = -1; rx < 2; rx++)
				for (ry = -1; ry < 2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r&0xFF) == PT_PPIP)
							parts[r>>8].flags = parts[i].flags; //Flood fill doesn't work with diagonal single pixel pipe, so spread the changes manually
					}
		}
	}
	if (parts[i].ctype>=2 && parts[i].ctype<=4 && !(parts[i].flags & PFLAG_PAUSE))
	{
		if (parts[i].life==3)
		{
			int lastneighbor = -1;
			int neighborcount = 0;
			int count = 0;
			// make automatic pipe pattern
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							continue;
						if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP)&&parts[r>>8].ctype==1)
						{
							parts[r>>8].ctype = (((parts[i].ctype)%3)+2);//reverse
							parts[r>>8].life = 6;
							if ( parts[i].tmp&0x100)//is a single pixel pipe
							{
								parts[r>>8].tmp |= 0x200;//will transfer to a single pixel pipe
								parts[r>>8].tmp |= count<<10;//coords of where it came from
								parts[i].tmp |= ((7-count)<<13);
							}
							neighborcount ++;
							lastneighbor = r>>8;
						}
						else if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP)&&parts[r>>8].ctype!=(((parts[i].ctype-1)%3)+2))
						{
							neighborcount ++;
							lastneighbor = r>>8;
						}
						count++;
					}
					if(neighborcount == 1)
						parts[lastneighbor].tmp |= 0x100;
		}
		else
		{
			if (parts[i].flags&PFLAG_NORMALSPEED)//skip particle push to prevent particle number being higher causing speed up
			{
				parts[i].flags &= ~PFLAG_NORMALSPEED;
			}
			else
			{
				pushParticle(i,0,i);
			}

			if (nt)//there is something besides PIPE around current particle
			{
				rndstore = rand();
				rnd = rndstore&7;
				rndstore = rndstore>>3;
				rx = pos_1_rx[rnd];
				ry = pos_1_ry[rnd];
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = pmap[y+ry][x+rx];
					if(!r)
						r = photons[y+ry][x+rx];
					if (surround_space && !r && (parts[i].tmp&0xFF)!=0)  //creating at end
					{
						np = create_part(-1,x+rx,y+ry,parts[i].tmp&0xFF);
						if (np!=-1)
						{
							parts[np].temp = parts[i].temp;//pipe saves temp and life now
							parts[np].life = parts[i].tmp2;
							parts[np].tmp = parts[i].pavg[0];
							parts[np].ctype = parts[i].pavg[1];
							parts[i].tmp &= ~0xFF;
						}
					}
					//try eating particle at entrance
					else if ((parts[i].tmp&0xFF) == 0 && (ptypes[r&0xFF].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						if ((r&0xFF)==PT_SOAP)
							detach(r>>8);
						parts[i].tmp =  (parts[i].tmp&~0xFF) | parts[r>>8].type;
						parts[i].temp = parts[r>>8].temp;
						parts[i].tmp2 = parts[r>>8].life;
						parts[i].pavg[0] = parts[r>>8].tmp;
						parts[i].pavg[1] = parts[r>>8].ctype;
						kill_part(r>>8);
					}
					else if ((parts[i].tmp&0xFF) == 0 && (r&0xFF)==PT_STOR && parts[r>>8].tmp && (ptypes[parts[r>>8].tmp].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY)))
					{
						parts[i].tmp =  parts[r>>8].tmp;
						parts[i].temp = parts[r>>8].temp;
						parts[i].tmp2 = parts[r>>8].tmp2;
						parts[i].pavg[0] = parts[r>>8].pavg[0];
						parts[i].pavg[1] = parts[r>>8].pavg[1];
						parts[r>>8].tmp = 0;
						parts[r>>8].life = 0;
					}
				}
			}
		}
	}
	else if (!parts[i].ctype && parts[i].life<=10)
	{
		if (parts[i].temp<272.15)//manual pipe colors
		{
			if (parts[i].temp>173.25&&parts[i].temp<273.15)
			{
				parts[i].ctype = 2;
				parts[i].life = 0;
			}
			if (parts[i].temp>73.25&&parts[i].temp<=173.15)
			{
				parts[i].ctype = 3;
				parts[i].life = 0;
			}
			if (parts[i].temp>=0&&parts[i].temp<=73.15)
			{
				parts[i].ctype = 4;
				parts[i].life = 0;
			}
		}
		else
		{
			// make a border
			for (rx=-2; rx<3; rx++)
				for (ry=-2; ry<3; ry++)
				{
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
						{
							int index = create_part(-1,x+rx,y+ry,PT_BRCK);//BRCK border, people didn't like DMND
							if (parts[i].type == PT_PPIP && index != -1)
								parts[index].tmp = 1;
						}
					}
				}
			if (parts[i].life<=1)
				parts[i].ctype = 1;
		}
	}
	else if (parts[i].ctype==1)//wait for empty space before starting to generate automatic pipe pattern
	{
		if (!parts[i].life)
		{
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						if (!pmap[y+ry][x+rx] && bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_ALLOWAIR && bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALL && bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_WALLELEC && (bmap[(y+ry)/CELL][(x+rx)/CELL]!=WL_EWALL || emap[(y+ry)/CELL][(x+rx)/CELL]))
							parts[i].life=50;
					}
		}
		else if (parts[i].life==5)//check for beginning of pipe single pixel
		{
			int issingle = 1;
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (((r&0xFF)==PT_PIPE || (r&0xFF) == PT_PPIP) && parts[i].ctype==1 && parts[i].life )
							issingle = 0;
					}
			if (issingle)
				parts[i].tmp |= 0x100;
		}
		else if (parts[i].life==2)
		{
			parts[i].ctype = 2;
			parts[i].life = 6;
		}
	}
	return 0;
}

int graphics_PIPE(GRAPHICS_FUNC_ARGS)
{

	if ((cpart->tmp&0xFF)>0 && (cpart->tmp&0xFF)<PT_NUM)
	{
		//Create a temp. particle and do a subcall.
		particle tpart;
		int t;
		memset(&tpart, 0, sizeof(particle));
		tpart.type = cpart->tmp&0xFF;
		tpart.temp = cpart->temp;
		tpart.life = cpart->tmp2;
		tpart.tmp = cpart->pavg[0];
		tpart.ctype = cpart->pavg[1];
		t = tpart.type;
		if (graphicscache[t].isready)
		{
			*pixel_mode = graphicscache[t].pixel_mode;
			*cola = graphicscache[t].cola;
			*colr = graphicscache[t].colr;
			*colg = graphicscache[t].colg;
			*colb = graphicscache[t].colb;
			*firea = graphicscache[t].firea;
			*firer = graphicscache[t].firer;
			*fireg = graphicscache[t].fireg;
			*fireb = graphicscache[t].fireb;
		}
		else
		{
			*colr = PIXR(ptypes[t].pcolors);
			*colg = PIXG(ptypes[t].pcolors);
			*colb = PIXB(ptypes[t].pcolors);
			if (ptypes[t].graphics_func)
			{
				(*(ptypes[t].graphics_func))(&tpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
			}
			else
			{
				graphics_DEFAULT(&tpart, nx, ny, pixel_mode, cola, colr, colg, colb, firea, firer, fireg, fireb);
			}
		}
		//*colr = PIXR(ptypes[cpart->tmp&0xFF].pcolors);
		//*colg = PIXG(ptypes[cpart->tmp&0xFF].pcolors);
		//*colb = PIXB(ptypes[cpart->tmp&0xFF].pcolors);
	}
	else
	{
		if (cpart->ctype==2)
		{
			*colr = 50;
			*colg = 1;
			*colb = 1;
		}
		else if (cpart->ctype==3)
		{
			*colr = 1;
			*colg = 50;
			*colb = 1;
		}
		else if (cpart->ctype==4)
		{
			*colr = 1;
			*colg = 1;
			*colb = 50;
		}
		else if (cpart->temp<272.15&&cpart->ctype!=1)
		{
			if (cpart->temp>173.25&&cpart->temp<273.15)
			{
				*colr = 50;
				*colg = 1;
				*colb = 1;
			}
			if (cpart->temp>73.25&&cpart->temp<=173.15)
			{
				*colr = 1;
				*colg = 50;
				*colb = 1;
			}
			if (cpart->temp>=0&&cpart->temp<=73.15)
			{
				*colr = 1;
				*colg = 1;
				*colb = 50;
			}
		}
	}
	return 0;
}
