#include <element.h>

int update_ACRN(UPDATE_FUNC_ARGS) {
	int r,rx,ry, t;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>8)>=NPART || !r)
                    continue;
                if((r&0xFF)!=PT_ACRN)
                {
                    if ((r&0xFF)==PT_PLEX || (r&0xFF)==PT_NITR || (r&0xFF)==PT_GUNP || (r&0xFF)==PT_RBDM || (r&0xFF)==PT_LRBD)
                    {
                        parts[i].type = PT_FIRE;
                        parts[i].life = 4;
                        parts[r>>8].type = PT_FIRE;
                        parts[r>>8].life = 4;
                    }
                    else if((((r&0xFF)!=PT_ACLOUD && (r&0xFF)!=PT_CLNE && (r&0xFF)!=PT_PCLN && (r&0xFF)!=PT_GSCL && (r&0xFF)!=PT_LQCL && ptypes[parts[r>>8].type].hardness>(rand()%1000)))&&parts[i].life>=50)
                    {
                        if(parts_avg(i, r>>8,PT_GLAS)!= PT_GLAS)
                        {
                            parts[i].life--;
                            parts[r>>8].type = PT_NONE;
                        }
                    }
                    else if (parts[i].life==50)
                    {
                        parts[i].life = 0;
                        parts[i].type = PT_NONE;
                    }
                }
            }
	return 0;
}
