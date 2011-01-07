#include <powder.h>

int update_FUSE(UPDATE_FUNC_ARGS) {
	int r;
	if (parts[i].life<=0) {
		//t = parts[i].life = PT_NONE;
		kill_part(i);
		r = create_part(-1, x, y, PT_PLSM);
		if (r!=-1)
			parts[r].life = 50;
		return 1;
	} else if (parts[i].life < 40) {
		parts[i].life--;
		if ((rand()%100)==0) {
			r = create_part(-1, (nx=x+rand()%3-1), (ny=y+rand()%3-1), PT_PLSM);
			if (r!=-1)
				parts[r].life = 50;
		}
	}
	if ((pv[y/CELL][x/CELL] > 2.7f)&&parts[i].tmp>40)
		parts[i].tmp=39;
	else if (parts[i].tmp<40&&parts[i].tmp>0)
		parts[i].tmp--;
	else if (parts[i].tmp<=0) {
		//t = PT_NONE;
		kill_part(i);
		r = create_part(-1, x, y, PT_FSEP);
		return 1;
	}
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_SPRK || ((parts[i].temp>=(273.15+700.0f)) && 1>(rand()%20)))
				{
					if (parts[i].life>40) {
						parts[i].life = 39;
					}
				}
			}
	return 0;
}
