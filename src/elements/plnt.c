#include <element.h>

int update_PLNT(UPDATE_FUNC_ARGS) {
	int r, nx, ny, np, t;
        for(nx=-2; nx<3; nx++)
            for(ny=-2; ny<3; ny++)
                if(x+nx>=0 && y+ny>0 &&
                   x+nx<XRES && y+ny<YRES && (nx || ny))
                {
                    r = pmap[y+ny][x+nx];
                    if((r>>8)>=NPART || !r)
                        continue;
                    if((r&0xFF)==PT_WATR && 1>(rand()%250))
                    {
                        parts[i].type = PT_PLNT;
                        parts[r>>8].type = PT_PLNT;
                        parts[r>>8].life = 0;
                    }
                    else if((r&0xFF)==PT_RWTR && 1>(rand()%250))
                    {
                        parts[i].type = PT_PLNT;
                        parts[r>>8].type = PT_PLNT;
                        parts[r>>8].life = 0;
                    }							
                    else if((r&0xFF)==PT_LAVA && 1>(rand()%250))
                    {
                        parts[i].life = 4;
                        parts[i].type = PT_FIRE;
                    }
                    else if((r&0xFF)==PT_SMKE && (1>rand()%250))
                    {
                        parts[r>>8].type = PT_NONE;
                        parts[i].life = rand()%60 + 60;
                    }
                    else if((r&0xFF)==PT_WOOD)
                    {
                        int nnx = rand()%3 -1;
                        int nny = rand()%3 -1;
                        if(x+nx+nnx>=0 && y+ny+nny>0 &&
                           x+nx+nnx<XRES && y+ny+nny<YRES && (nnx || nny))
                        {
                            if((pmap[y+ny+nny][x+nx+nnx]>>8)>=NPART||pmap[y+ny+nny][x+nx+nnx])
                                continue;
                            if(create_part(-1,x+nx+nnx,y+ny+nny,PT_VINE))
                                parts[pmap[y+ny+nny][x+nx+nnx]>>8].temp = parts[i].temp;
                        }
                    }
                    // && (1>rand()%20) && abs(nx+ny)<=2 && VINE_MODE
                    else if((r&0xFF)==PT_DIRT)
                    {
                        int nnx = rand()%3 -1;
                        int nny = rand()%3 -1;
                        if(x+nx+nnx>=0 && y+ny+nny>0 &&
                           x+nx+nnx<XRES && y+ny+nny<YRES && (nnx || nny))
                        {
                            if((pmap[y+ny+nny][x+nx+nnx]>>8)>=NPART||pmap[y+ny+nny][x+nx+nnx])
                                continue;
                            if(create_part(-1,x+nx+nnx,y+ny+nny,PT_GRAS))
                                parts[pmap[y+ny+nny][x+nx+nnx]>>8].temp = parts[i].temp;
                        }
                    }
                    
                    //if(t==PT_SNOW && (r&0xFF)==PT_WATR && 15>(rand()%1000))
                    //t = parts[i].type = PT_WATR;
                }
		if(parts[i].life==2)
		{
		    for(nx=-1; nx<2; nx++)
                for(ny=-1; ny<2; ny++)
                    if(x+nx>=0 && y+ny>0 &&
                       x+nx<XRES && y+ny<YRES && (nx || ny))
                    {
                        r = pmap[y+ny][x+nx];
                        if((r>>8)>=NPART)
                            continue;
                        if(!r)
                            create_part(-1,x+nx,y+ny,PT_O2);
                    }
		    parts[i].life = 0;
		}
	return 0;
}
