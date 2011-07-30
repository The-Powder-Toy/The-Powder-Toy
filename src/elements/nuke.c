#include <element.h>

int update_NUKE(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
	int self = parts[i].type;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[r>>PS].type==PT_FIRE || parts[r>>PS].type==PT_PLSM || parts[r>>PS].type==PT_BFLM || parts[r>>PS].type==PT_NCWS){
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
                    part_change_type(r>>PS,x+rx,y+ry,PT_NCWS);
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
