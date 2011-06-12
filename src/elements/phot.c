#include <element.h>

int update_PHOT(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry;
	float rr, rrr;
    if (!parts[i].ctype) {
        kill_part(i);
        return 1;
        }
	if (1>rand()%10) update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry)) {
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if ((r&TYPE)==PT_ISOZ && 5>(rand()%2000))
				{
					parts[i].vx *= 0.90;
					parts[i].vy *= 0.90;
					create_part(r>>PS, x+rx, y+ry, PT_PHOT);
					rrr = (rand()%360)*3.14159f/180.0f;
					rr = (rand()%128+128)/127.0f;
					parts[r>>PS].vx = rr*cosf(rrr);
					parts[r>>PS].vy = rr*sinf(rrr);
					pv[y/CELL][x/CELL] -= 15.0f * CFDS;
				}
				if ((r&TYPE)==PT_ISZS && 5>(rand()%2000))
				{
					parts[i].vx *= 0.90;
					parts[i].vy *= 0.90;
					create_part(r>>PS, x+rx, y+ry, PT_PHOT);
					rr = (rand()%228+128)/127.0f;
					rrr = (rand()%360)*3.14159f/180.0f;
					parts[r>>PS].vx = rr*cosf(rrr);
					parts[r>>PS].vy = rr*sinf(rrr);
					pv[y/CELL][x/CELL] -= 15.0f * CFDS;
				}
			}
	r = pmap[y][x];
	rt = (r&TYPE);
	if (rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN) {
		if (!parts[r>>PS].ctype)
			parts[r>>PS].ctype = PT_PHOT;
	}

	return 0;
}
