#include <element.h>

int update_CFIR(UPDATE_FUNC_ARGS) {
	int r,rx,ry, trade;
	for ( trade = 0; trade<10; trade ++)
	{
		rx = rand()%3-1;
		ry = rand()%3-1;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if ((r>>PS)>=NPART || !r)
				continue;
			if ((r&TYPE)!=PT_WARP&&(r&TYPE)!=PT_POT&&(r&TYPE)!=PT_STKM&&(r&TYPE)!=PT_STKM2&&(r&TYPE)!=PT_DMND&&(r&TYPE)!=PT_CLNE&&(r&TYPE)!=PT_BCLN&&(r&TYPE)!=PT_PCLN&&(10>=rand()%2))
			{
                int nrx = rand()%3 -1;
                int nry = rand()%3 -1;
                if(x+rx+nrx>=0 && y+ry+nry>0 &&
                   x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                {
                    if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                        continue;
                    if (r && (r&TYPE)!=PT_CFIR){
                        create_part(-1, x+rx+nrx, y+ry+nry, PT_WARP);
                    } else if (parts[r>>PS].type==PT_CFIR){
                        parts[i].x = rand()%3;
                        parts[i].x = rand()%-3;
                        parts[i].y = rand()%3;
                        parts[i].y = rand()%-3;
                    } else {
                        parts[i].vy = 2;
                    }
                }
				parts[i].life += 100;
				pmap[y][x] = r;
				pmap[y+ry][x+rx] = (i<<PS)|parts[i].type;
                create_part(-1, x, y, PT_CFIR);
			}
        }
	}
    return 0;
}
