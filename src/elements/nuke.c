#include <element.h>

int update_NUKE(UPDATE_FUNC_ARGS) {
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
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);
                    parts[r>>8].type = PT_NCWS;
                    parts[r>>8].life = 128;
                }
                if ((r&0xFF)==PT_PLSM){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);
                    parts[r>>8].type = PT_NCWS;
                    parts[r>>8].life = 128;
                }
                if ((r&0xFF)==PT_BFLM){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);                    
                    parts[r>>8].type = PT_NCWS;
                    parts[r>>8].life = 128;
                }
                if ((r&0xFF)==PT_SPRK){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);
                }
            }
    return 0;
}
