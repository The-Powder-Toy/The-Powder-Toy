#include <powder.h>

int update_PHOT(UPDATE_FUNC_ARGS) {
	if (1>rand()%10) update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
	int r, rt;
	float rr, rrr;
	for (nx=0; nx<1; nx++)
		for (ny=0; ny<1; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_WATR || (r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW)
				{
					parts[i].vx *= 0.995;
					parts[i].vy *= 0.995;
				}
			}
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if ((r&0xFF)==PT_ISOZ && 5>(rand()%2000))
				{
					parts[i].vx *= 0.90;
					parts[i].vy *= 0.90;
					parts[r>>8].type = PT_PHOT;
					rrr = (rand()%360)*3.14159f/180.0f;
					rr = (rand()%128+128)/127.0f;
					parts[r>>8].life = 680;
					parts[r>>8].ctype = 0x3FFFFFFF;
					parts[r>>8].vx = rr*cosf(rrr);
					parts[r>>8].vy = rr*sinf(rrr);
					pv[y/CELL][x/CELL] -= 15.0f * CFDS;
				}
				if ((r&0xFF)==PT_ISZS && 5>(rand()%2000))
				{
					parts[i].vx *= 0.90;
					parts[i].vy *= 0.90;
					parts[r>>8].type = PT_PHOT;
					rr = (rand()%228+128)/127.0f;
					rrr = (rand()%360)*3.14159f/180.0f;
					parts[r>>8].life = 680;
					parts[r>>8].ctype = 0x3FFFFFFF;
					parts[r>>8].vx = rr*cosf(rrr);
					parts[r>>8].vy = rr*sinf(rrr);
					pv[y/CELL][x/CELL] -= 15.0f * CFDS;
				}
			}

	r = pmap[ny][nx];
	rt = r&0xFF;
	if (rt==PT_CLNE || rt==PT_PCLN || rt==PT_BCLN) {

		if (!parts[r>>8].ctype)
			parts[r>>8].ctype = PT_PHOT;
	}

	// TODO

	return 0;
}
