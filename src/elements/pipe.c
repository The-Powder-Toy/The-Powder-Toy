#include <element.h>

signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

void pushParticle(int i, int count, int original)
{
	int rndstore, rnd, rx, ry, r, x, y, np, q, notctype=(((parts[i].ctype)%3)+2);
	int self = parts[i].type;
	if ((parts[i].tmp&TYPE) == 0 || count >= 2)//don't push if there is nothing there, max speed of 2 per frame
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
				if ((r>>PS)>=NPART || !r)
					continue;
				else if ((r&TYPE)==self && parts[r>>PS].ctype!=notctype && (parts[r>>PS].tmp&TYPE)==0)
				{
					parts[r>>PS].tmp = (parts[r>>PS].tmp&~TYPE) | (parts[i].tmp&TYPE);
					parts[r>>PS].temp = parts[i].temp;
					parts[r>>PS].flags = parts[i].flags;
					parts[r>>PS].pavg[0] = parts[i].pavg[0];
					parts[r>>PS].pavg[1] = parts[i].pavg[1];
					if (r>>PS > original)
						parts[r>>PS].tmp2 = 1;//skip particle push, normalizes speed
					parts[i].tmp &= ~TYPE;
					count++;
					pushParticle(r>>PS,count,original);
				}
			}
		}
	}
	else //predefined 1 pixel thick pipe movement
	{
		int coords = 7 - (parts[i].tmp>>10);
		r = pmap[y+ pos_1_ry[coords]][x+ pos_1_rx[coords]];
		if ((r>>PS)>=NPART || !r)
		{
		}
		else if ((r&TYPE)==self && parts[r>>PS].ctype!=notctype && (parts[r>>PS].tmp&TYPE)==0)
		{
			parts[r>>PS].tmp = (parts[r>>PS].tmp&~TYPE) | (parts[i].tmp&TYPE);
			parts[r>>PS].temp = parts[i].temp;
			parts[r>>PS].flags = parts[i].flags;
			parts[r>>PS].pavg[0] = parts[i].pavg[0];
			parts[r>>PS].pavg[1] = parts[i].pavg[1];
			if (r>>PS > original)
				parts[r>>PS].tmp2 = 1;//skip particle push, normalizes speed
			parts[i].tmp &= ~TYPE;
			count++;
			pushParticle(r>>PS,count,original);
		}


	}
	return;
}

int update_PIPE(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	int self = parts[i].type;
	int rnd, rndstore;
	if (parts[i].ctype>=2 && parts[i].ctype<=4)
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
						if ((r>>PS)>=NPART || !r)
							continue;
						if ((r&TYPE)==self&&parts[r>>PS].ctype==1)
						{
							parts[r>>PS].ctype = (((parts[i].ctype)%3)+2);//reverse
							parts[r>>PS].life = 6;
							if ( parts[i].tmp&0x100)//is a single pixel pipe
							{
								parts[r>>PS].tmp |= 0x200;//will transfer to a single pixel pipe
								parts[r>>PS].tmp |= count<<10;//coords of where it came from
							}
							neighborcount ++;
							lastneighbor = r>>PS;
						}
						else if ((r&TYPE)==self&&parts[r>>PS].ctype!=(((parts[i].ctype-1)%3)+2))
						{
							neighborcount ++;
							lastneighbor = r>>PS;
						}
						count++;
					}
					if(neighborcount == 1)
						parts[lastneighbor].tmp |= 0x100;
		}
		else
		{
			if (parts[i].tmp2 == 1)//skip particle push to prevent particle number being higher causeing speed up
			{
				parts[i].tmp2 = 0 ;
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
					if ((r>>PS)>=NPART)
						return 0;
					if (surround_space && !r && (parts[i].tmp&TYPE)!=0)  //creating at end
					{
						np = create_part(-1,x+rx,y+ry,parts[i].tmp&TYPE);
						if (np!=-1)
						{
							parts[np].temp = parts[i].temp;//pipe saves temp and life now
							parts[np].life = parts[i].flags;
							parts[np].tmp = parts[i].pavg[0];
							parts[np].ctype = parts[i].pavg[1];
						}
						parts[i].tmp &= ~TYPE;
					}
					//try eating particle at entrance
					else if ((parts[i].tmp&TYPE) == 0 && (parts[r>>PS].falldown!= 0 || ptypes[r&TYPE].state == ST_GAS))
					{
						parts[i].tmp =  (parts[i].tmp&~TYPE) | parts[r>>PS].type;
						parts[i].temp = parts[r>>PS].temp;
						parts[i].flags = parts[r>>PS].life;
						parts[i].pavg[0] = parts[r>>PS].tmp;
						parts[i].pavg[1] = parts[r>>PS].ctype;
						kill_part(r>>PS);
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
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if (!r)
							create_part(-1,x+rx,y+ry,PT_BRCK);//BRCK border, people didn't like DMND
					}
					if (parts[i].life==1)
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
						if (!pmap[y+ry][x+rx])
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
						if ((r&TYPE)==self && parts[i].ctype==1 && parts[i].life )
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
