#include <element.h>

int update_IRON(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int self = parts[i].type;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (((parts[r>>PS].type == PT_SALT && 15>(rand()/(RAND_MAX/700))) ||
				        (parts[r>>PS].type == PT_SLTW && 30>(rand()/(RAND_MAX/2000))) ||
				        (parts[r>>PS].type == PT_WATR && 5 >(rand()/(RAND_MAX/6000))) ||
				        (parts[r>>PS].type == PT_O2   && 2 >(rand()/(RAND_MAX/500))) ||
				        (parts[r>>PS].type == PT_LO2))&&
				        (!(parts[i].life))
				   )
				{
					part_change_type(i,x,y,PT_BMTL);
					parts[i].tmp=(rand()/(RAND_MAX/10))+20;
				}
			}
	return 0;
}
