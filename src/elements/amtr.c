#include <element.h>

int update_AMTR(UPDATE_FUNC_ARGS) {
    int self = parts[i].type;
	int r, rx, ry;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts[r>>PS].type!=PT_AMTR && parts[r>>PS].type!=self && parts[r>>PS].type!=PT_DMND && parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_PCLN && parts[r>>PS].type!=PT_NONE && parts[r>>PS].type!=PT_PHOT && parts[r>>PS].type!=PT_VOID && parts[r>>PS].type!=PT_BHOL && parts[r>>PS].type!=PT_NBHL && parts[r>>PS].type!=PT_PRTI && parts[r>>PS].type!=PT_PRTO)
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
