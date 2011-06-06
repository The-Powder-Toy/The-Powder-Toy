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
                    if (!r || (r>>PS)>=NPART)
                        r = pmap[y+ry][x+rx];
                    if (!r || (r>>PS)>=NPART)
                        continue;
                    if (parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_PCLN && parts[r>>PS].type!=PT_C0 &&
                    parts[r>>PS].type!=PT_BCLN && parts[r>>PS].type!=PT_STKM &&
                    parts[r>>PS].type!=PT_STKM2 && parts[r>>PS].type!=PT_PDCL &&
                    parts[r>>PS].type!=PT_GSCL && parts[r>>PS].type!=PT_LQCL && parts[r>>PS].type<PT_NUM)
                        parts[i].ctype = parts[r>>PS].type;
                    if(parts[r>>PS].type==PT_C0 && parts[r>>PS].ctype!=0){
                        parts[i].ctype = parts[r>>PS].ctype;
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
                    if (!r || (r>>PS)>=NPART)
                        r = pmap[y+ry][x+rx];
                    if (!r || (r>>PS)>=NPART)
                        continue;
                    if (pv[y/CELL][x/CELL]>5.0f)
                    {
                        if (1>rand()%6)
                        {
                            part_change_type(i,x,y,parts[i].ctype);
                            parts[r>>PS].temp = parts[i].temp;
                            parts[i].life = rand()%150+50;
                            pv[y/CELL][x/CELL] += parts[i].tmp2;
                        }
                    }

                }
	}
	return 0;
}
