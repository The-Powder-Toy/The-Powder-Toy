#include <element.h>

int update_GRAS(UPDATE_FUNC_ARGS) {
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
                        parts[r>>PS].type = self;
                        parts[r>>PS].life = 0;
                    }
                    else if(parts[r>>PS].type==PT_SMKE)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        parts[r>>PS].type = PT_NONE;
                        parts[i].life = rand()%60 + 60;
                        create_part(-1,x+rx+nrx,y+ry+nry,PT_O2);
                    }
                    else if (parts[r>>PS].type==PT_WOOD && (1>rand()%20) && abs(rx+ry)<=2 && VINE_MODE)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        if(x+rx+nrx>=0 && y+ry+nry>0 &&
                           x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                        {
                            if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                continue;
                            if(create_part(-1,x+rx+nrx,y+ry+nry,PT_VINE))
                                parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                        }
                    }
                    else if(parts[r>>PS].type==PT_RWTR && 1>(rand()%250))
                    {
                        t = parts[i].type = self;
                        parts[r>>PS].type = self;
                        parts[r>>PS].life = 0;
                    }
                    else if(parts[r>>PS].type==PT_LAVA && 1>(rand()%250))
                    {
                        parts[i].life = 4;
                        t = parts[i].type = PT_FIRE;
                    }
                    else if(parts[r>>PS].type==PT_O3)
                    {
                        t = parts[i].type = PT_FIRE;
                    }
                    else if(parts[r>>PS].type==PT_ROOT && parts[r>>PS].tmp2==1)
                    {
                        parts[r>>PS].tmp2 = 2;
                        parts[i].ctype = PT_ROOT;
                    }
                    if (parts[i].ctype==PT_ROOT)
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        if (parts[r>>PS].type == self && parts[r>>PS].ctype != PT_ROOT && (1>rand()%50))
                            parts[r>>PS].ctype = PT_ROOT;

                        if(create_part(-1,x+rx+nrx,y+ry+nry,self))
                            parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                        if (1>rand()%2)
                            parts[i].ctype=PT_NONE;
                    }
                    else if(parts[r>>PS].type==self)
                    {
                        if(rand()%8000 < 1){
                            int nrx = rand()%3 -1;
                            //int nry = rand()%3 -1;
                            int nry = -1;
                            if(x+rx+nrx>=0 && y+ry+nry>0 &&
                               x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                            {
                                if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                    continue;
                                if(create_part(-1,x+rx+nrx,y+ry+nry,self))
                                    parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                            }
                        }
                    }
                    else if(parts[r>>PS].type==PT_DIRT && (1>rand()%20))
                    {
                        int nrx = rand()%3 -1;
                        int nry = rand()%3 -1;
                        if(x+rx+nrx>=0 && y+ry+nry>0 &&
                           x+rx+nrx<XRES && y+ry+nry<YRES && (nrx || nry))
                        {
                            if((pmap[y+ry+nry][x+rx+nrx]>>PS)>=NPART||pmap[y+ry+nry][x+rx+nrx])
                                continue;
                            if(create_part(-1,x+rx+nrx,y+ry+nry,self))
                                parts[pmap[y+ry+nry][x+rx+nrx]>>PS].temp = parts[i].temp;
                        }
                        if (rand()%800 < 1){
                            parts[r>>PS].type = PT_ROOT;
                            parts[r>>PS].tmp2=0;
                        }
                    }


                }
	return 0;
}
