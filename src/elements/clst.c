#include <element.h>

int update_CLST(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	float cxy;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if ((r&0xFF)==PT_WATR && 1>(rand()%1500))
				{
					part_change_type(i,x,y,PT_PSTS);					
					kill_part(r>>8);
				}
				if ((r&0xFF)==PT_NITR)
				{
					create_part(i, x, y, PT_BANG);
					create_part(r>>8, x+rx, y+ry, PT_BANG);
				}
				if ((r&0xFF)==PT_CLST)
				{
					if(parts[i].temp <195)
						cxy = 0.05;
					if(parts[i].temp >= 195 && parts[i].temp <295)
						cxy = 0.015;
					if(parts[i].temp >= 295 && parts[i].temp <350)
						cxy = 0.01;
					if(parts[i].temp > 350)
						cxy = 0.005;
					parts[i].vx += cxy*rx;
					parts[i].vy += cxy*ry;//These two can be set not to calculate over 350 later. They do virtually nothing over 0.005.
				}
			}
	return 0;
}
