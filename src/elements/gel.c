#include <element.h>

int update_GEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, trade, np;
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if ((r>>PS)>=NPART)
						continue;
					if (!r)
                    {
                        if (parts[i].vx > 3)
                            parts[i].vx = 3;
                        if (parts[i].vx < -3)
                            parts[i].vx = -3;
                        if (parts[i].vy > 3)
                            parts[i].vy = 3;
                        if (parts[i].vy < -3)
                            parts[i].vy = -3;
                    }
                    else
                    {
                        if (parts[i].vx > 0.1)
                            parts[i].vx = 0.1;
                        if (parts[i].vx < -0.1)
                            parts[i].vx = -0.1;
                        if (parts[i].vy > 0.1)
                            parts[i].vy = 0.1;
                        if (parts[i].vy < -0.1)
                            parts[i].vy = -0.1;
                    }
				}

	return 0;
}
