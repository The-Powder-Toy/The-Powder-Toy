#include <element.h>

int update_AMTR(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE)!=PT_AMTR && (r&TYPE)!=PT_DMND && (r&TYPE)!=PT_CLNE && (r&TYPE)!=PT_PCLN && (r&TYPE)!=PT_NONE && (r&TYPE)!=PT_PHOT && (r&TYPE)!=PT_VOID && (r&TYPE)!=PT_BHOL && (r&TYPE)!=PT_NBHL && (r&TYPE)!=PT_PRTI && (r&TYPE)!=PT_PRTO)
				{
					parts[i].life++;
					if (parts[i].life==4)
					{
						kill_part(i);
						return 1;
					}
					if (10>(rand()/(RAND_MAX/100)))
						create_part(r>>PS, x+rx, y+ry, PT_PHOT);
					else
						kill_part(r>>PS);
					pv[y/CELL][x/CELL] -= 2.0f;
				}
			}
	return 0;
}
