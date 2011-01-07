#include <powder.h>

int update_BMTL(UPDATE_FUNC_ARGS) {
	int r, rt;
	if (parts[i].tmp>1) {
		parts[i].tmp--;
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r)
						continue;
					rt =parts[r>>8].type;
					if ((rt==PT_METL || rt==PT_IRON) && 1>(rand()/(RAND_MAX/100)))
					{
						parts[r>>8].type=PT_BMTL;
						parts[r>>8].tmp=(parts[i].tmp<=7)?parts[i].tmp=1:parts[i].tmp-(rand()%5);//rand()/(RAND_MAX/300)+100;
					}
				}
	} else if (parts[i].tmp==1 && 1>rand()%1000) {
		parts[i].tmp = 0;
		parts[i].type = PT_BRMT;
	}
	return 0;
}
