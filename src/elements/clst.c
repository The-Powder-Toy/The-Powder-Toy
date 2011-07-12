#include <element.h>

int update_CLST(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float cxy;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>(rand()%1500))
				{
					part_change_type(i,x,y,PT_PSTS);					
					kill_part(r>>8);
				}
				if ((r&0xFF)==PT_CLST)
				{
				        if(parts[i].temp >= 0 && parts[i].temp <294)
					cxy = 0.02;
				        if(parts[i].temp >= 295 && parts[i].temp <349)
					cxy = 0.01;
				        if(parts[i].temp >= 250 && parts[i].temp <499)
					cxy = 0.005;
				        if(parts[i].temp >= 500)
					cxy = 0.001;
					parts[i].vx += cxy*rx;
					parts[i].vy += cxy*ry;
				}
			}
	return 0;
}
