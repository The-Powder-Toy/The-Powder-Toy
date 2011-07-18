#include <element.h>

int update_IFIL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	int self = parts[i].type;
	if (parts[i].tmp>1) {
		parts[i].tmp--;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
					rt = parts[r>>PS].type;
					if ((rt==PT_METL || rt==PT_IRON) && 1>(rand()/(RAND_MAX/100)))
					{
						part_change_type(r>>PS,x+rx,y+ry,self);
						parts[r>>PS].tmp=(parts[i].tmp<=7)?parts[i].tmp=1:parts[i].tmp-(rand()%5);//rand()/(RAND_MAX/300)+100;
					}
				}
	} else if (parts[i].tmp==1 && 1>rand()%1000) {
		parts[i].tmp = 0;
		part_change_type(i,x,y,self);
	}
	return 0;
}
