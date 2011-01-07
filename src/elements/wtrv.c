#include <powder.h>

int update_WTRV(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_WATR||(r&0xFF)==PT_DSTW||(r&0xFF)==PT_SLTW) && 1>(rand()%1000) && legacy_enable)
				{
					parts[i].type = PT_WATR;
					parts[r>>8].type = PT_WATR;
				}

				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && parts[i].temp>12.0f && 1>(rand()%500))
				{
					parts[i].life = 4;
					parts[i].type = PT_FIRE;

				}
				if (((r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) && 1>(rand()%1000) && legacy_enable)
				{
					parts[i].type = PT_WATR;
					if (1>(rand()%1000))
						parts[r>>8].type = PT_WATR;
				}
			}
	return 0;
}
