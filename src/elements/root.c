#include <element.h>

int update_ROOT(UPDATE_FUNC_ARGS) {
	int r,rx,ry,t;
        for(rx=-2; rx<3; rx++)
            for(ry=-2; ry<3; ry++)
                if(x+rx>=0 && y+ry>0 &&
                   x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if((r>>8)>=NPART || !r)
                        continue;
                    if((r&0xFF)==PT_WATR && 1>(rand()%250))
                    {
                        t = parts[i].type = PT_ROOT;
                        parts[r>>8].type = PT_PLNT;
                        parts[r>>8].life = 0;
                    }
                    else if((r&0xFF)==PT_RWTR && 1>(rand()%250))
                    {
                        t = parts[i].type = PT_ROOT;
                        parts[r>>8].type = PT_PLNT;
                        parts[r>>8].life = 0;
                    }							
                    else if((r&0xFF)==PT_O3)
                    {
                        t = parts[i].type = PT_FIRE;
                    }
                    else if((r&0xFF)==PT_DIRT)
                    {
                        if (rand()%8000 < 1){
                            if (parts[r>>8].type == PT_DIRT){
                                if (parts[r>>8].type != PT_ROOT){
                                    if (parts[r>>8].type != PT_GRAS){
                                        parts[r>>8].type = PT_ROOT;
                                    }
                                }
                            }
                        }
                    }
                    
                    
                }
	return 0;
}
