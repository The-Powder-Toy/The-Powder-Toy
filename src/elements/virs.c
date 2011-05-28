#include <element.h>

int update_VIRS(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 &&
               x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if((r&TYPE)!=PT_DMND||(r&TYPE)!=PT_GOLD||(r&TYPE)!=PT_CLNE||(r&TYPE)!=PT_CNVR||(r&TYPE)!=PT_VOID||(r&TYPE)!=PT_WHOL||(r&TYPE)!=PT_BHOL)
                    parts[r>>PS].type=PT_VIRS;
            }
    return 0;
}
