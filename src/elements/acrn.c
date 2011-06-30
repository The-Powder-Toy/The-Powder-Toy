#include <element.h>

int update_ACRN(UPDATE_FUNC_ARGS) {
	int r,rx,ry, t;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if(parts[r>>PS].type!=PT_ACRN)
                {
                    if (parts[r>>PS].type==PT_PLEX || parts[r>>PS].type==PT_NITR || parts[r>>PS].type==PT_GUNP || parts[r>>PS].type==PT_RBDM || parts[r>>PS].type==PT_LRBD)
                    {
                        parts[i].type = PT_FIRE;
                        parts[i].life = 4;
                        parts[r>>PS].type = PT_FIRE;
                        parts[r>>PS].life = 4;
                    }
                    else if(((parts[r>>PS].type!=PT_ACLOUD && parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_PCLN && parts[r>>PS].type!=PT_GSCL && parts[r>>PS].type!=PT_LQCL && ptypes[parts[r>>PS].type].hardness>(rand()%1000)))&&parts[i].life>=50)
                    {
                        if(parts_avg(i, r>>PS,PT_GLAS)!= PT_GLAS)
                        {
                            parts[i].life--;
                            parts[r>>PS].type = PT_NONE;
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
