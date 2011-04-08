#include <element.h>

int update_TNT(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>8)>=NPART || !r)
                    continue;
                if ((r&0xFF)==PT_FIRE){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_FIRE);
                        create_part(-1,x-1,y-1,PT_FIRE);
                        create_part(-1,x-1,y+1,PT_FIRE);
                        create_part(-1,x+1,y-1,PT_FIRE);
                        create_part(-1,x,y,PT_PAIN);
                        temporaro--;
                    }
                    parts[i].type = PT_FIRE;
                    parts[i].life = 128;
                    parts[r>>8].type = PT_FIRE;
                    parts[r>>8].life = 128;
                }
            }
    return 0;
}
