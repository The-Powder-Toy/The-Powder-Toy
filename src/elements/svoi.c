#include <element.h>

int update_SVOI(UPDATE_FUNC_ARGS) {
    int self = parts[i].type;
	if (!parts[i].ctype)
	{
		int rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_CLNE &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_CNVR &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_BCLN &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_PDCL &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_GSCL &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_LQCL &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_STKM2 &&
                        (pmap[y+ry][x+rx]&TYPE)!=self &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_VOID &&
				        (pmap[y+ry][x+rx]&TYPE)!=TYPE)
                        parts[i].ctype = pmap[y+ry][x+rx]&TYPE;
	}
	else {
        int r, rx, ry;
        for (rx=-1; rx<2; rx++)
            for (ry=-1; ry<2; ry++)
                if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if ((r>>PS)>=NPART || !r)
                        continue;
                    if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES &&
				        pmap[y+ry][x+rx] &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_CLNE &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_CNVR &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_BCLN &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_PDCL &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_GSCL &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_LQCL &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_PCLN &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_STKM &&
				        (pmap[y+ry][x+rx]&TYPE)!=PT_STKM2 &&
                        (pmap[y+ry][x+rx]&TYPE)!=self &&
                        (pmap[y+ry][x+rx]&TYPE)!=PT_VOID &&
				        (pmap[y+ry][x+rx]&TYPE)!=TYPE){
                        if (parts[r>>PS].type == parts[i].ctype){
                            parts[r>>PS].type = PT_NONE;
                        }
                    }
                    if (parts[r>>PS].type==self){
                        parts[r>>PS].ctype=parts[i].ctype;
                    }
                }
	}
	return 0;
}
