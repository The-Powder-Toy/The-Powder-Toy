#include <element.h>

int update_GRAS(UPDATE_FUNC_ARGS) {
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
                        t = parts[i].type = PT_GRAS;
                        parts[r>>8].type = PT_GRAS;
                        parts[r>>8].life = 0;
                    }
                    else if((r&0xFF)==PT_SMKE)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        parts[r>>8].type = PT_NONE;
                        parts[i].life = rand()%60 + 60;
                        create_part(-1,x+rx+nrx,y+ry+nry,PT_O2);
                    }
                    else if((r&0xFF)==PT_WOOD)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        if(x+rx+nrx>=0 && y+ry+nry>0 &&
                           x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                        {
                            if((pmap[y+ry+nry][x+rx+nrx]>>8)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                continue;
                            if(create_part(-1,x+rx+nrx,y+ry+nry,PT_VINE))
                                parts[pmap[y+ry+nry][x+rx+nrx]>>8].temp = parts[i].temp;
                        }
                    }
                    else if((r&0xFF)==PT_RWTR && 1>(rand()%250))
                    {
                        t = parts[i].type = PT_GRAS;
                        parts[r>>8].type = PT_GRAS;
                        parts[r>>8].life = 0;
                    }							
                    else if((r&0xFF)==PT_LAVA && 1>(rand()%250))
                    {
                        parts[i].life = 4;
                        t = parts[i].type = PT_FIRE;
                    }
                    else if((r&0xFF)==PT_O3)
                    {
                        t = parts[i].type = PT_FIRE;
                    }
                    else if((r&0xFF)==PT_GRAS)
                    {
                        if(rand()%8000 < 1){
                            int nrx = rand()%3 -1;
                            //int nry = rand()%3 -1;
                            int nry = -1;
                            if(x+rx+nrx>=0 && y+ry+nry>0 &&
                               x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                            {
                                if((pmap[y+ry+nry][x+rx+nrx]>>8)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                    continue;
                                if(create_part(-1,x+rx+nrx,y+ry+nry,PT_GRAS))
                                    parts[pmap[y+ry+nry][x+rx+nrx]>>8].temp = 0;
                            }
                        }
                    }
                    else if((r&0xFF)==PT_DIRT)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        if(x+rx+nrx>=0 && y+ry+nry>0 &&
                           x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                        {
                            if((pmap[y+ry+nry][x+rx+nrx]>>8)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                continue;
                            if(create_part(-1,x+rx+nrx,y+ry+nry,PT_GRAS))
                                parts[pmap[y+ry+nry][x+rx+nrx]>>8].temp = 0;
                        }
                        if (rand()%8000 < 1){
                            if (parts[r>>8].type==PT_DIRT){
                                parts[r>>8].type = PT_ROOT;
                            }
                        }
                    }
                    
                    
                }
	return 0;
}
