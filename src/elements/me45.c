#include <element.h>

int update_ME45(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for(rx=-1; rx<2; rx++)
		for(ry=-1; ry<2; ry++)
			if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && pmap[y+ry][x+rx] && (pmap[y+ry][x+rx]&0xFF)!=0xFF) {
				r = pmap[y+ry][x+rx];
				parts[i].ctype = rand()%PT_NUM+1;
				if (parts[i].ctype != PT_STKM &&
				parts[i].ctype != PT_STKM2 &&
				parts[i].ctype != PT_SPAWN &&
				parts[i].ctype != PT_SPAWN2 &&
				parts[i].ctype != PT_EQUALVEL) {
					if (parts[i].ctype == PT_LIFE) {
						parts[i].tmp = rand()%NGT_BRAN+NGT_GOL;
						create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype|(parts[i].tmp<<8));
					}
					else {
						create_part(-1, x+rand()%3-1, y+rand()%3-1, parts[i].ctype);
					}
				}

			}
	return 0;
}