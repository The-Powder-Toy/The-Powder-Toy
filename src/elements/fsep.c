#include <element.h>

int update_FSEP(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life<=0) {
		r = create_part(i, x, y, PT_PLSM);
		if (r!=-1)
			parts[r].life = 50;
		return 1;
	} else if (parts[i].life < 40) {
		parts[i].life--;
		if ((rand()%10)==0) {
			r = create_part(-1, (rx=x+rand()%3-1), (ry=y+rand()%3-1), PT_PLSM);
			if (r!=-1)
				parts[r].life = 50;
		}
	}
	else {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_SPRK || (parts[i].temp>=(273.15+400.0f))) && 1>(rand()%15))
					{
						if (parts[i].life>40) {
							parts[i].life = 39;
						}
					}
				}
	}
	return 0;
}
