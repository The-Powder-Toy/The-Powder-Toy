#include <powder.h>

int update_CLNE(UPDATE_FUNC_ARGS) {
	if (!parts[i].ctype)
	{
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 &&
				        x+nx<XRES && y+ny<YRES &&
				        pmap[y+ny][x+nx] &&
				        (pmap[y+ny][x+nx]&0xFF)!=PT_CLNE &&
				        (pmap[y+ny][x+nx]&0xFF)!=PT_BCLN &&
				        (pmap[y+ny][x+nx]&0xFF)!=PT_PCLN &&
				        (pmap[y+ny][x+nx]&0xFF)!=PT_STKM &&
				        (pmap[y+ny][x+nx]&0xFF)!=PT_STKM2 &&
				        (pmap[y+ny][x+nx]&0xFF)!=0xFF)
					parts[i].ctype = pmap[y+ny][x+nx]&0xFF;
	}
	else {
		create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
	}
	return 0;
}
