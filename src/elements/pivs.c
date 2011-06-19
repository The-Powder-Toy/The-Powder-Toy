#include <element.h>

int update_PIVS(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (parts[i].life>0 && parts[i].life!=10)
        parts[i].life--;
	if (parts[i].life==10)
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
					if ((r&TYPE)==PT_PIVS)
					{
						if (parts[r>>PS].life<10&&parts[r>>PS].life>0)
							parts[i].life = 9;
						else if (parts[r>>PS].life==0)
							parts[r>>PS].life = 10;
					}
				}
	}
	return 0;
}
