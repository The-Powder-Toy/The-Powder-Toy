#include <powder.h>

int update_FSEP(UPDATE_FUNC_ARGS) {
	int r;
	if (parts[i].life<=0) {
		//t = PT_NONE;
		kill_part(i);
		r = create_part(-1, x, y, PT_PLSM);
		if (r!=-1)
			parts[r].life = 50;
		return 1;
	} else if (parts[i].life < 40) {
		parts[i].life--;
		if ((rand()%10)==0) {
			r = create_part(-1, (nx=x+rand()%3-1), (ny=y+rand()%3-1), PT_PLSM);
			if (r!=-1)
				parts[r].life = 50;
		}
	}
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_SPRK || (parts[i].temp>=(273.15+400.0f))) && 1>(rand()%15))
				{
					if (parts[i].life>40) {
						parts[i].life = 39;
					}
				}
			}
	return 0;
}
