#include <powder.h>

int update_SLTW(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10) && legacy_enable)
				{
					parts[i].type = PT_SALT;
					parts[r>>8].type = PT_WTRV;
				}
				else if ((r&0xFF)==PT_SALT && 1>(rand()%10000))
				{
					parts[r>>8].type = PT_NONE;
				}
				if ((r&0xFF)==PT_PLNT&&5>(rand()%1000))
					parts[r>>8].type = PT_NONE;
				if (((r&0xFF)==PT_RBDM||(r&0xFF)==PT_LRBD) && parts[i].temp>12.0f && 1>(rand()%500))
				{
					parts[i].life = 4;
					parts[i].type = PT_FIRE;

				}
			}
	return 0;
}
