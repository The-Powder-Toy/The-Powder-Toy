#include <element.h>

int update_COOK(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	int t = parts[i].temp - 273.15;	
	for (rx=-2; rx<3; rx++)
		for (ry=-2; ry<3; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (parts[i].life < t) 
				{ 
					if (t < 0)
						t = 0;
					if (t > 600)
						t = 600;
					parts[i].life = t;
				}
			}
	return 0;
}
