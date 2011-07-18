#include <element.h>

int update_PRTN(UPDATE_FUNC_ARGS) {
	int r, rt, rx, ry, tmpo;
	int self = parts[i].type;
	float rr, rrr;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES && (rx || ry)) {
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
                if (parts[i].life<10){
                    tmpo = 1000;
                    while (tmpo > 0){
                        rt = create_parts(x,y,rx,ry,PT_NEUT);
                        parts[rt].life = rand()%480+480;
                        tmpo--;
                    }
                }
            }
	r = pmap[y][x];
	rt = r&TYPE;
	if (rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN) {
		if (!parts[r>>PS].ctype)
			parts[r>>PS].ctype = PT_PHOT;
	}
	return 0;
}
