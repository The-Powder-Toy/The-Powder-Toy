#include <element.h>

int update_VIRS(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 &&
               x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>8)>=NPART || !r)
                    continue;
                if((r&0xFF)!=PT_DMND||(r&0xFF)!=PT_GOLD||(r&0xFF)!=PT_CLNE||(r&0xFF)!=PT_CNVR||(r&0xFF)!=PT_VOID||(r&0xFF)!=PT_WHOL||(r&0xFF)!=PT_BHOL)
                    parts[r>>8].type=PT_VIRS;
            }
    return 0;
}
