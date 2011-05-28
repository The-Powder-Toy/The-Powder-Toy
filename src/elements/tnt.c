#include <element.h>

int update_TNT(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if ((r&TYPE)==PT_FIRE){
                    pv[y/CELL][x/CELL] += 15.0f;
                    if (y+CELL<YRES)
                        pv[y/CELL+1][x/CELL] += 15.0f;
                    if (x+CELL<XRES)
                    {
                        pv[y/CELL][x/CELL+1] += 15.0f;
                        if (y+CELL<YRES)
                            pv[y/CELL+1][x/CELL+1] += 15.0f;
                    }
                    parts[i].type = PT_NONE;
                }
                if ((r&TYPE)==PT_PLSM){
                    pv[y/CELL][x/CELL] += 15.0f;
                    if (y+CELL<YRES)
                        pv[y/CELL+1][x/CELL] += 15.0f;
                    if (x+CELL<XRES)
                    {
                        pv[y/CELL][x/CELL+1] += 15.0f;
                        if (y+CELL<YRES)
                            pv[y/CELL+1][x/CELL+1] += 15.0f;
                    }
                    parts[i].type = PT_NONE;
                }
                if ((r&TYPE)==PT_BFLM){
                    pv[y/CELL][x/CELL] += 15.0f;
                    if (y+CELL<YRES)
                        pv[y/CELL+1][x/CELL] += 15.0f;
                    if (x+CELL<XRES)
                    {
                        pv[y/CELL][x/CELL+1] += 15.0f;
                        if (y+CELL<YRES)
                            pv[y/CELL+1][x/CELL+1] += 15.0f;
                    }
                    parts[i].type = PT_NONE;
                }
                if ((r&TYPE)==PT_SPRK){
                    pv[y/CELL][x/CELL] += 15.0f;
                    if (y+CELL<YRES)
                        pv[y/CELL+1][x/CELL] += 15.0f;
                    if (x+CELL<XRES)
                    {
                        pv[y/CELL][x/CELL+1] += 15.0f;
                        if (y+CELL<YRES)
                            pv[y/CELL+1][x/CELL+1] += 15.0f;
                    }
                    parts[i].type = PT_NONE;
                }
            }
    return 0;
}
