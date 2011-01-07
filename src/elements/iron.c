#include <powder.h>

int update_IRON(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((((r&0xFF) == PT_SALT && 15>(rand()/(RAND_MAX/700))) ||
				        ((r&0xFF) == PT_SLTW && 30>(rand()/(RAND_MAX/2000))) ||
				        ((r&0xFF) == PT_WATR && 5 >(rand()/(RAND_MAX/6000))) ||
				        ((r&0xFF) == PT_O2   && 2 >(rand()/(RAND_MAX/500))) ||
				        ((r&0xFF) == PT_LO2))&&
				        (!(parts[i].life))
				   )
				{
					parts[i].type=PT_BMTL;
					parts[i].tmp=(rand()/(RAND_MAX/10))+20;
				}
			}
	return 0;
}
