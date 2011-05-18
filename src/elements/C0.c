#include <element.h>

int update_C0(UPDATE_FUNC_ARGS) {
	if (!parts[i].ctype)
	{
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
                {
                    r = photons[y+ry][x+rx];
                    if (!r || (r>>8)>=NPART)
                        r = pmap[y+ry][x+rx];
                    if (!r || (r>>8)>=NPART)
                        continue;
                    if ((r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_C0 &&
                    (r&0xFF)!=PT_BCLN && (r&0xFF)!=PT_STKM &&
                    (r&0xFF)!=PT_STKM2 && (r&0xFF)!=PT_PDCL &&
                    (r&0xFF)!=PT_GSCL && (r&0xFF)!=PT_LQCL && (r&0xFF)<PT_NUM)
                        parts[i].ctype = r&0xFF;
                    if(parts[r>>8].type==PT_C0 && parts[r>>8].ctype!=0){
                        parts[i].ctype = parts[r>>8].ctype;
                    }
                }
	}
	else {
	    int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>=0 && x+rx<XRES && y+ry<YRES)
                {
                    r = photons[y+ry][x+rx];
                    if (!r || (r>>8)>=NPART)
                        r = pmap[y+ry][x+rx];
                    if (!r || (r>>8)>=NPART)
                        continue;
                    if (pv[y/CELL][x/CELL]>5.0f)
                    {
                        if (1>rand()%6)
                        {
                            part_change_type(i,x,y,parts[i].ctype);
                            parts[r>>8].temp = parts[i].temp;
                            parts[i].life = rand()%150+50;
                            pv[y/CELL][x/CELL] += parts[i].tmp2;
                        }
                    }

                }
	}
	return 0;
}
