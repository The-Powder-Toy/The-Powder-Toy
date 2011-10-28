#include <element.h>

int update_ICDR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for(rx=-1; rx<2; rx++)
		for(ry=-1; ry<2; ry++)
			if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && pmap[y+ry][x+rx] && (pmap[y+ry][x+rx]&0xFF)!=0xFF) {
				r = pmap[y+ry][x+rx];
				if (parts[r>>8].type == PT_SIMN) {
					parts[r>>8].type = PT_CCTV;
				}
			}
	return 0;
}