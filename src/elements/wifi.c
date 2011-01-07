#include <powder.h>

int update_WIFI(UPDATE_FUNC_ARGS) {
	int r, temp, temprange = 100;
	for ( temp = 0; temp < MAX_TEMP; temp += temprange)
		if (parts[i].temp-73.15>temp&&parts[i].temp-73.15 <temp+temprange)
			parts[i].tmp = temp/100;
	for (ny=-1; ny<2; ny++)
		for (nx=-1; nx<2; nx++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (parts[r>>8].type==PT_NSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
				{
					parts[r>>8].type = PT_SPRK;
					parts[r>>8].ctype = PT_NSCN;
					parts[r>>8].life = 4;
				}
				else if (parts[r>>8].type==PT_PSCN&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
				{
					parts[r>>8].type = PT_SPRK;
					parts[r>>8].ctype = PT_PSCN;
					parts[r>>8].life = 4;
				}
				else if (parts[r>>8].type==PT_INWR&&parts[r>>8].life==0 && wireless[parts[i].tmp][0])
				{
					parts[r>>8].type = PT_SPRK;
					parts[r>>8].ctype = PT_INWR;
					parts[r>>8].life = 4;
				}
				else if (parts[r>>8].type==PT_SPRK && parts[r>>8].ctype!=PT_NSCN && parts[r>>8].life>=3 && !wireless[parts[i].tmp][0])
				{
					//parts[r>>8].type = parts[r>>8].ctype;
					wireless[parts[i].tmp][0] = 1;
					wireless[parts[i].tmp][1] = 1;
					ISWIRE = 1;
				}
			}
	return 0;
}
