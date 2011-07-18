#include <element.h>

int update_ROOT(UPDATE_FUNC_ARGS) {
	int r,rx,ry,t;
	int self = parts[i].type;
        for(rx=-2; rx<3; rx++)
            for(ry=-2; ry<3; ry++)
                if(x+rx>=0 && y+ry>0 &&
                   x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if((r>>PS)>=NPART || !r)
                        continue;
                    if(parts[r>>PS].type==PT_WATR && 1>(rand()%250))
                    {
                        t = parts[i].type = self;
                        parts[r>>PS].type = PT_NONE;
                        parts[i].tmp2 = 1;
                    }
                    else if(parts[r>>PS].type==PT_RWTR && 1>(rand()%250))
                    {
                        t = parts[i].type = self;
                        parts[r>>PS].type = PT_NONE;
                        parts[i].tmp2 = 1;
                    }
                    else if(parts[r>>PS].type==PT_O3)
                    {
                        t = parts[i].type = PT_FIRE;
                    }
                    else if(parts[r>>PS].type==PT_DIRT)
                    {
                        if (rand()%8000 < 1){
                            if (parts[r>>PS].type == PT_DIRT){
                                if (parts[r>>PS].type != self){
                                    if (parts[r>>PS].type != PT_GRAS){
                                        parts[r>>PS].type = self;
                                    }
                                }
                            }
                        }
                    }
                    if (parts[i].tmp2==1 && parts[r>>PS].tmp2!=1 && parts[r>>PS].tmp2!=2){
                        if (parts[r>>PS].type==self){
                                parts[r>>PS].tmp2=1;
                                parts[i].tmp2=0;
                        }
                    }
                    if (parts[r>>PS].tmp2==2 && 1>(rand()%5)){
                        parts[r>>PS].tmp2=0;
                    }


                }
	return 0;
}
