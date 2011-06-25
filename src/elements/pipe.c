#include <element.h>

signed char pos_1_rx[] = {-1,-1,-1, 0, 0, 1, 1, 1};
signed char pos_1_ry[] = {-1, 0, 1,-1, 1,-1, 0, 1};

int update_PIPE(UPDATE_FUNC_ARGS) {
	int r, rx, ry, np;
	if (parts[i].ctype>=2 && parts[i].ctype<=4)
	{
		if (parts[i].life==3)
		{
			// make automatic pipe pattern
			for (rx=-1; rx<2; rx++)
				for (ry=-1; ry<2; ry++)
					if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
					{
						r = pmap[y+ry][x+rx];
						if ((r>>PS)>=NPART || !r)
							continue;
						if ((r&TYPE)==PT_PIPE&&parts[r>>PS].ctype==1)
						{
							parts[r>>PS].ctype = (((parts[i].ctype)%3)+2);//reverse
							parts[r>>PS].life = 6;
						}
					}
		}
		else
		{
			int rndstore, rnd, q, ctype=(((parts[i].ctype)%3)+2);
			rndstore = rand();
			// RAND_MAX is at least 32767 on all platforms i.e. pow(8,5)-1
			// so can go 5 cycles without regenerating rndstore
			for (q=0; q<3; q++)
			{
				rnd = rndstore&7;
				rndstore = rndstore>>3;
				rx = pos_1_rx[rnd];
				ry = pos_1_ry[rnd];
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART)
						continue;
					else if (!r&&parts[i].tmp!=0)
					{
						np = create_part(-1,x+rx,y+ry,parts[i].tmp);
						if (np!=-1)
						{
							parts[np].temp = parts[i].temp;//pipe saves temp and life now
							parts[np].life = parts[i].flags;
							parts[np].tmp = parts[i].tmp2;
							parts[np].ctype = parts[i].tmpx;
						}
						parts[i].tmp = 0;
						continue;
					}
					else if (!r)
						continue;
					else if (parts[i].tmp == 0 && (ptypes[r&TYPE].falldown!= 0 || ptypes[r&TYPE].state == ST_GAS))
					{
						parts[i].tmp = parts[r>>PS].type;
						parts[i].temp = parts[r>>PS].temp;
						parts[i].flags = parts[r>>PS].life;
						kill_part(r>>PS);
					}
					else if ((r&TYPE)==PT_PIPE && parts[r>>PS].ctype!=ctype && parts[r>>PS].tmp==0&&parts[i].tmp>0)
					{
						parts[r>>PS].tmp = parts[i].tmp;
						parts[r>>PS].temp = parts[i].temp;
						parts[r>>PS].flags = parts[i].flags;
						parts[i].tmp = 0;
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
		else if (parts[i].life==2)
		{
			parts[i].ctype = 2;
			parts[i].life = 6;
		}
	}
	return 0;
}
