#include <powder.h>

int update_ICEI(UPDATE_FUNC_ARGS) { //currently used for snow as well
	int r;
	if (parts[i].ctype==PT_FRZW)
	{
		parts[i].temp -= 1.0f;
		if (parts[i].temp<0)
			parts[i].temp = 0;

	}
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_SALT || (r&0xFF)==PT_SLTW) && 1>(rand()%1000))
				{
					parts[i].type = PT_SLTW;
					parts[r>>8].type = PT_SLTW;
				}
				if (legacy_enable)
				{
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
					{
						parts[i].type = PT_ICEI;
						parts[r>>8].type = PT_ICEI;
					}
					if (parts[i].type==PT_SNOW && ((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 15>(rand()%1000))
						parts[i].type = PT_WATR;
				}
			}
	return 0;
}
