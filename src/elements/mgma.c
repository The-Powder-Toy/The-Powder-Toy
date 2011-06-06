#include <element.h>

int update_MGMA(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 &&
               x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if(rand()%8000 < 1){
                    int nnx = rand()%3 -1;
                    //int nny = rand()%3 -1;
                    int nny = -1;
                    if(x+rx+nnx>=0 && y+ry+nny>0 &&
                       x+rx+nnx<XRES && y+ry+nny<YRES && (nnx || nny))
                    {
                        if((pmap[y+ry+nny][x+rx+nnx]>>PS)>=NPART||pmap[y+ry+nny][x+rx+nnx])
                            continue;
                        if(create_part(-1,x+rx+nnx,y+ry+nny,PT_SMKE))
                            parts[pmap[y+ry+nny][x+rx+nnx]>>PS].temp = parts[i].temp;
                    }
                }

            }
    if(rand()%1000 < 1)
        parts[i].temp += 50;
    return 0;
}
