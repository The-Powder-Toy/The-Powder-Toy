#include <element.h>

int update_H2(UPDATE_FUNC_ARGS)
{
    int r,rx,ry,rt,nx,ny;
    for(rx=-2; rx<3; rx++)
        for(ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
                rt = (r&0xFF);
				if ((r>>8)>=NPART || !r)
					continue;
                if(pv[y/CELL][x/CELL] > 8.0f && rt == PT_DESL)
                {
                    part_change_type(r,nx,ny,PT_WATR);
                    part_change_type(i,x,y,PT_OIL);
                }
            }
}
