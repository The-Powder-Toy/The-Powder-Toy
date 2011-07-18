#include <element.h>

int update_TNT(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
	int self = parts[i].type;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_SPRK || parts[r>>PS].type==PT_BFLM || parts[r>>PS].type==PT_PLSM || parts[r>>PS].type==PT_FIRE){
                    pv[y/CELL][x/CELL] += 15.0f;
                    if (y+CELL<YRES)
                        pv[y/CELL+1][x/CELL] += 15.0f;
                    if (x+CELL<XRES)
                    {
                        pv[y/CELL][x/CELL+1] += 15.0f;
                        if (y+CELL<YRES)
                            pv[y/CELL+1][x/CELL+1] += 15.0f;
                    }
                    kill_part(i);
                }
            }
    return 0;
}
