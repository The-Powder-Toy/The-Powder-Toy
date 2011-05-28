#include <element.h>

int update_LAZR(UPDATE_FUNC_ARGS) {
	int r,rx,ry,t;
        for(rx=-2; rx<3; rx++)
            for(ry=-2; ry<3; ry++)
                if(x+rx>=0 && y+ry>0 &&
                   x+rx<XRES && y+ry<YRES && (rx || ry))
                {
                    r = pmap[y+ry][x+rx];
                    if((r>>PS)>=NPART || !r)
                        continue;
                        if(rand()%4000 < 1){
                            int nrx = rand()%3 -1;
                            //int nry = rand()%3 -1;
                            int nry = -1;
                            if(x+rx+nrx>=0 && y+ry+nry>0 &&
                               x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                            {
                                if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                    continue;
                                if(create_part(-1,x+rx+nrx,y+ry+nry,PT_SMKE))
                                    parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = 0;
                            }
                        }

                }
	return 0;
}
