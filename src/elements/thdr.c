#include <powder.h>

int update_THDR(UPDATE_FUNC_ARGS) {
	int r;
	for (nx=-2; nx<3; nx++)
		for (ny=-2; ny<3; ny++)
			if (x+nx>=0 && y+ny>0 &&
			        x+nx<XRES && y+ny<YRES && (nx || ny))
			{
				r = pmap[y+ny][x+nx];
				if ((r>>8)>=NPART || !r)
					continue;
				if (((r&0xFF)==PT_METL || (r&0xFF)==PT_IRON || (r&0xFF)==PT_ETRD || (r&0xFF)==PT_PSCN || (r&0xFF)==PT_NSCN || (r&0xFF)==PT_NTCT || (r&0xFF)==PT_PTCT || (r&0xFF)==PT_BMTL || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD || (r&0xFF)==PT_BRMT || (r&0xFF)==PT_NBLE || (r&0xFF)==PT_INWR) && parts[r>>8].ctype!=PT_SPRK)
				{
					parts[i].type = PT_NONE;
					parts[r>>8].ctype = parts[r>>8].type;
					parts[r>>8].type = PT_SPRK;
					parts[r>>8].life = 4;
				}
				else if ((r&0xFF)!=PT_CLNE&&(r&0xFF)!=PT_THDR&&(r&0xFF)!=PT_SPRK&&(r&0xFF)!=PT_DMND&&(r&0xFF)!=PT_FIRE&&(r&0xFF)!=PT_NEUT&&(r&0xFF)!=PT_PHOT&&(r&0xFF))
				{
					pv[y/CELL][x/CELL] += 100.0f;
					if (legacy_enable&&1>(rand()%200))
					{
						parts[i].life = rand()%50+120;
						parts[i].type = PT_FIRE;
					}
					else
					{
						parts[i].type = PT_NONE;
					}
				}
			}
	return 0;
}
