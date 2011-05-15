#include <element.h>

int update_NMTR(UPDATE_FUNC_ARGS) {
    int r,rx,ry;
	if (parts[i].tmp)
		parts[i].tmp--;
	if (parts[i].temp>2500)
	{
		rx=rand()%3-1;
		ry=rand()%3-1;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && !pmap[y+ry][x+rx] && (!((r>>8)>=NPART)))
		{
			int dp=create_part(-1,x+rx,y+ry,PT_PLSM);
			parts[dp].temp=MAX_TEMP;
		}
		parts[i].temp=MAX_TEMP;
		part_change_type(i,x,y,PT_PLSM);
		parts[i].life=rand()%250+50;
		pv[y/CELL][x/CELL] += 50.0f * CFDS;
	}
        if (ngrav_enable==1){
            gravmap[y/CELL][x/CELL] += 0.01f;
        }
	return 0;
}
