#include <element.h>

int update_PLAN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
            if(x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
            {
                r = pmap[y+ry][x+rx];
                if((r>>PS)>=NPART || !r)
                    continue;
                if (parts[i].name){
                    parts[r>>PS].name = parts[i].name;
                } else {
                    parts[i].name = input_ui(vid_buf, "Planet Name", "Enter planet name", "Earth", "255");
                }
            }

	if (ngrav_enable==1){
        gravmap[y/CELL][x/CELL] += parts[i].tmp;
    }
	return 0;
}
