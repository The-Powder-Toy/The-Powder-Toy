#include <powder.h>

int update_AMTR(UPDATE_FUNC_ARGS) {
	int r, rt;
	for (nx=-1; nx<2; nx++)
		for (ny=-1; ny<2; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				rt = parts[r>>8].type;
				if ((r&0xFF)!=PT_AMTR && (r&0xFF)!=PT_DMND && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_NONE && (r&0xFF)!=PT_PHOT && (r&0xFF)!=PT_VOID && (r&0xFF)!=PT_BHOL && (r&0xFF)!=PT_PRTI && (r&0xFF)!=PT_PRTO)
				{
					parts[i].life++;
					if (parts[i].life==3)
					{
						parts[i].type = PT_NONE;
						kill_part(i);
						return 1;
					}
					parts[r>>8].life = 0;
					parts[r>>8].type = PT_NONE;
					kill_part(r>>8);
					if (2>(rand()/(RAND_MAX/100)))
						create_part(-1, x+nx, y+ny, PT_PHOT);
					pv[y/CELL][x/CELL] -= 5.0f;
				}
			}
	return 0;
}
