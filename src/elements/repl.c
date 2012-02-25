#include <element.h>

int update_REPL(UPDATE_FUNC_ARGS) {
	int r, rx, ry, ri;
	for(ri = 0; ri <= 10; ri++)
	{
		rx = (rand()%20)-10;
		ry = (rand()%20)-10;
		if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
		{
			r = pmap[y+ry][x+rx];
			if (!r)
				r = photons[y+ry][x+rx];

			if (r && !(ptypes[r&0xFF].properties & TYPE_SOLID)){
				parts[r>>8].vx += isign(rx)*((parts[i].temp-273.15)/10.0f);
				parts[r>>8].vy += isign(ry)*((parts[i].temp-273.15)/10.0f);
			}
		}
	}
	return 0;
}
