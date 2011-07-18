#include <element.h>

int update_LCRY(UPDATE_FUNC_ARGS) {
    int self = parts[i].type;
	if (parts[i].life>0 && parts[i].life!=10)
		parts[i].life--;
	if (parts[i].life==10)
	{
		int r, rx, ry;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART || !r)
						continue;
					if (parts[r>>PS].type==self)
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
