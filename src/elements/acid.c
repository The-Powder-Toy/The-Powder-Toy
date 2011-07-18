#include <element.h>

int update_ACID(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
	int self = parts[i].type;
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
				if (parts[r>>PS].type!=self)
				{
					if (parts[r>>PS].type==PT_PLEX || parts[r>>PS].type==PT_NITR || parts[r>>PS].type==PT_GUNP || parts[r>>PS].type==PT_RBDM || parts[r>>PS].type==PT_CSIM || parts[r>>PS].type==PT_LRBD)
					{
						part_change_type(i,x,y,PT_FIRE);
						part_change_type(r>>PS,x+rx,y+ry,PT_FIRE);
						parts[i].life = 4;
						parts[r>>PS].life = 4;
					}
					else if ((parts[r>>PS].type!=PT_CLNE && parts[r>>PS].type!=PT_PCLN && parts[r>>PS].hardness>(rand()%1000))&&parts[i].life>=50)
					{
						if (parts_avg(i, r>>PS,PT_GLAS)!= PT_GLAS)//GLAS protects stuff from acid
						{
						    float newtemp = ((60.0f-(float)parts[r>>PS].hardness))*7.0f;
                            if(newtemp < 0){
                                newtemp = 0;
                            }
                            parts[i].temp += newtemp;
							parts[i].life--;
							kill_part(r>>PS);
						}
					}
					else if (parts[i].life<=50)
					{
						kill_part(i);
						return 1;
					}
				}
			}
	return 0;
}
