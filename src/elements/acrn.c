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
                if((r&TYPE)!=PT_ACRN)
                {
                    if ((r&TYPE)==PT_PLEX || (r&TYPE)==PT_NITR || (r&TYPE)==PT_GUNP || (r&TYPE)==PT_RBDM || (r&TYPE)==PT_LRBD)
                    {
                        parts[i].type = PT_FIRE;
                        parts[i].life = 4;
                        parts[r>>PS].type = PT_FIRE;
                        parts[r>>PS].life = 4;
                    }
                    else if((((r&TYPE)!=PT_ACLOUD && (r&TYPE)!=PT_CLNE && (r&TYPE)!=PT_PCLN && (r&TYPE)!=PT_GSCL && (r&TYPE)!=PT_LQCL && ptypes[parts[r>>PS].type].hardness>(rand()%1000)))&&parts[i].life>=50)
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
