#include <element.h>

int update_VIRS(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
	int self = parts[i].type;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 &&
               x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if(parts[r>>PS].type!=PT_DMND||parts[r>>PS].type!=PT_GOLD||parts[r>>PS].type!=PT_CLNE||parts[r>>PS].type!=PT_CNVR||parts[r>>PS].type!=PT_VOID||parts[r>>PS].type!=PT_WHOL||parts[r>>PS].type!=PT_BHOL)
                    parts[r>>PS].type=self;
            }
    return 0;
}
