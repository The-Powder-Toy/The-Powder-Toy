#include <element.h>

int update_NCGN(UPDATE_FUNC_ARGS) {
	int r, nx, ny, rt;
    for(nx=-1; nx<2; nx++)
        for(ny=-1; ny<2; ny++)
            if(x+nx>=0 && y+ny>0 &&
               x+nx<XRES && y+ny<YRES && (nx || ny))
            {
                
                r = pmap[y+ny][x+nx];
                //if ((r&0xFF)==PT_PLUT){
                if (parts[r>>8].type==PT_PLUT){
                    create_part(-1, nx, ny,PT_NCWS);
                    rt = 3 + (int)pv[y/CELL][x/CELL];
                    for(nx=-12; nx<12; nx++)
                        for(ny=-12; ny<12; ny++)
                            if(x+nx>=0 && y+ny>0 &&
                               x+nx<XRES && y+ny<YRES && (nx || ny))
                            {
                                r = pmap[y+ny][x+nx];
                                if((r>>8)>=NPART || !r)
                                    continue;
                                rt = parts[r>>8].type;
                                if(parts_avg(i,r>>8,PT_INSL) != PT_INSL)
                                {
                                    if((rt==PT_METL||rt==PT_GOLD||rt==PT_IRON||rt==PT_ETRD||rt==PT_BMTL||rt==PT_BRMT||rt==PT_LRBD||rt==PT_RBDM||rt==PT_PSCN||rt==PT_NSCN||rt==PT_NBLE)&&parts[r>>8].life==0 && abs(nx)+abs(ny) < 4)
                                    {
                                        parts[r>>8].life = 4;
                                        parts[r>>8].ctype = rt;
                                        parts[r>>8].type = PT_SPRK;
                                    }
                                }
                            }
                }
                
            }
	return 0;
}
