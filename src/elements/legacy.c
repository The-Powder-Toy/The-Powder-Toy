#include <element.h>

// Interactions which only occur when legacy_enable is on
int update_legacy_all(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt;
	int t = parts[i].type;
	if (!legacy_enable) return 0;
	if (t==PT_WTRV) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR||(r&0xFF)==PT_DSTW||(r&0xFF)==PT_SLTW) && 1>(rand()%1000))
					{
						part_change_type(i,x,y,PT_WATR);
						part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					}
					if (((r&0xFF)==PT_ICEI || (r&0xFF)==PT_SNOW) && 1>(rand()%1000))
					{
						part_change_type(i,x,y,PT_WATR);
						if (1>(rand()%1000))
							part_change_type(r>>8,x+rx,y+ry,PT_WATR);
					}
				}
	}
	else if (t==PT_WATR) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
					{
						part_change_type(i,x,y,PT_WTRV);
					}
				}
	}
	else if (t==PT_SLTW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
					{
						part_change_type(i,x,y,PT_SALT);
						part_change_type(r>>8,x+rx,y+ry,PT_WTRV);
					}
				}
	}
	else if (t==PT_DSTW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 &&
				        x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_FIRE || (r&0xFF)==PT_LAVA) && 1>(rand()%10))
					{
						part_change_type(i,x,y,PT_WTRV);
					}
				}
	}
	else if (t==PT_ICEI) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
					{
						part_change_type(i,x,y,PT_ICEI);
						part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
					}
				}
	}
	else if (t==PT_SNOW) {
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 1>(rand()%1000))
					{
						part_change_type(i,x,y,PT_ICEI);
						part_change_type(r>>8,x+rx,y+ry,PT_ICEI);
					}
					if (((r&0xFF)==PT_WATR || (r&0xFF)==PT_DSTW) && 15>(rand()%1000))
						part_change_type(i,x,y,PT_WATR);
				}
	}
	if (t==PT_WTRV && pv[y/CELL][x/CELL]>4.0f)
		part_change_type(i,x,y,PT_DSTW);
	if (t==PT_OIL && pv[y/CELL][x/CELL]<-6.0f)
		part_change_type(i,x,y,PT_GAS);
	if (t==PT_GAS && pv[y/CELL][x/CELL]>6.0f)
		part_change_type(i,x,y,PT_OIL);
	if (t==PT_DESL && pv[y/CELL][x/CELL]>12.0f)
	{
		part_change_type(i,x,y,PT_FIRE);
		parts[i].life = rand()%50+120;
	}
	return 0;
}
