#include <element.h>

int update_NCGN(UPDATE_FUNC_ARGS) {
	int r, nx, ny, rt;
    for(nx=-1; nx<2; nx++)
        for(ny=-1; ny<2; ny++)
            if(x+nx>=0 && y+ny>0 &&
               x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>8)>=NPART || !r)
                    continue;
                if (parts[r>>8].type==PT_PLUT){
                    parts[i].ctype=PT_NCGN;
                    parts[i].life = 4;
                    parts[i].type = PT_SPRK;
                    parts[r>>8].type = PT_NCWS;
                }
            }
	return 0;
}
