#include <element.h>

int update_NUKE(UPDATE_FUNC_ARGS) {
	int r,nx,ny;
	int self = parts[i].type;
    for(nx=-2; nx<3; nx++)
        for(ny=-2; ny<3; ny++)
            if(x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                r = pmap[y+ny][x+nx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_FIRE || parts[r>>PS].type==PT_PLSM || parts[r>>PS].type==PT_BFLM){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        if (rand()%100 < 1)
                            create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);
                    parts[r>>PS].type = PT_NCWS;
                    parts[r>>PS].life = 128;
                    kill_part(i);
                }
                if (parts[r>>PS].type==PT_SPRK){
                    int temporaro = 1000;
                    while (temporaro > -1001){
                        create_part(-1,x+1,y+1,PT_NCWS);
                        create_part(-1,x-1,y-1,PT_PLUT);
                        create_part(-1,x-1,y+1,PT_NCWS);
                        create_part(-1,x+1,y-1,PT_PLUT);
                        if (rand()%100 < 1)
                            create_part(-1,x,y,PT_ZAP);
                        temporaro--;
                    }
                    create_part(-1,x,y,PT_PAIN);
                    kill_part(i);
                }
            }
    return 0;
}
