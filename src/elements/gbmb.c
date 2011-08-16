#include <element.h>
int update_GBMB(UPDATE_FUNC_ARGS) {
	int rx,ry,r;
	for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++){
				r = pmap[y+ry][x+rx];
				if(r)
				if((r&0xFF)!=PT_BOMB&&
				   (r&0xFF)!=PT_GBMB&&
				   (r&0xFF)!=PT_CLNE&&
				   (r&0xFF)!=PT_PCLN&&
				   (r&0xFF)!=PT_DMND&&
				   !parts[i].tmp){					
					parts[i].life=60;
					parts[i].tmp = 1;
				}
				}
	if(parts[i].life>20)
	gravmap[y/CELL][x/CELL] = 20;
	if(parts[i].life<20 && parts[i].life>=1)
	gravmap[y/CELL][x/CELL] = -80;
return 0;
}
