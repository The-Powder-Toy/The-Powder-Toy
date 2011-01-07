#include <powder.h>

int update_PIPE(UPDATE_FUNC_ARGS) {
	int r, trade, q, ctype;
	if (!parts[i].ctype && parts[i].life<=10)
	{
		if (parts[i].temp<272.15)
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
			for (nx=-2; nx<3; nx++)
				for (ny=-2; ny<3; ny++)
					if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
					{
						r = pmap[y+ny][x+nx];
						if ((r>>8)>=NPART )
							continue;
						if (!r)
							create_part(-1,x+nx,y+ny,PT_BRCK);
					}
			if (parts[i].life==1)
				parts[i].ctype = 1;
		}
	}
	if (parts[i].ctype==1)
	{
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART)
						continue;
					if (!r&&!parts[i].life)
						parts[i].life=50;
				}
		if (parts[i].life==2)
		{
			parts[i].ctype = 2;
			parts[i].life = 6;
		}
	}
	if (parts[i].ctype>1)
		for ( q = 0; q<3; q++)
			for ( ctype = 2; ctype<5; ctype++)
			{
				if (parts[i].ctype==ctype)
				{
					if (parts[i].life==3)
					{
						for (nx=-1; nx<2; nx++)
							for (ny=-1; ny<2; ny++)
								if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
								{
									r = pmap[y+ny][x+nx];
									if ((r>>8)>=NPART || !r)
										continue;
									if (parts[r>>8].type==PT_PIPE&&parts[r>>8].ctype==1)
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
						if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
						{
							r = pmap[y+ny][x+nx];
							if ((r>>8)>=NPART)
								continue;
							else if (!r&&parts[i].tmp!=0)
							{
								if (create_part(-1,x+nx,y+ny,parts[i].tmp))
								{
									parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;//pipe saves temp and life now
									parts[pmap[y+ny][x+nx]>>8].life = parts[i].flags;
								}
								parts[i].tmp = 0;
								continue;
							}
							else if (!r)
								continue;
							else if (parts[i].tmp == 0 && (ptypes[parts[r>>8].type].falldown!= 0 || pstates[parts[r>>8].type].state == ST_GAS))
							{
								parts[i].tmp = parts[r>>8].type;
								parts[i].temp = parts[r>>8].temp;
								parts[i].flags = parts[r>>8].life;
								parts[r>>8].type = PT_NONE;
							}
							else if (parts[r>>8].type==PT_PIPE && parts[r>>8].ctype!=(((ctype)%3)+2) && parts[r>>8].tmp==0&&parts[i].tmp>0)
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
	return 0;
}
