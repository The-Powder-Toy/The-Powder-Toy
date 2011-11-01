#include <element.h>

int update_ACEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && !(rx && ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = photons[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if(ptypes[r&0xFF].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[r>>8].vx *= 1.1f;
					parts[r>>8].vy *= 1.1f;
					parts[i].tmp = 1;
				}
			}
	return 0;
}

int graphics_ACEL(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}
int update_DCEL(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	parts[i].tmp = 0;
	for (rx=-1; rx<2; rx++)
		for (ry=-1; ry<2; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry) && !(rx && ry))
			{
				r = pmap[y+ry][x+rx];
				if(!r)
					r = photons[y+ry][x+rx];
				if ((r>>8)>=NPART || !r)
					continue;
				if(ptypes[r&0xFF].properties & (TYPE_PART | TYPE_LIQUID | TYPE_GAS | TYPE_ENERGY))
				{
					parts[r>>8].vx *= 0.9f;
					parts[r>>8].vy *= 0.9f;
					parts[i].tmp = 1;
				}
			}
	return 0;
}

int graphics_DCEL(GRAPHICS_FUNC_ARGS)
{
	if(cpart->tmp)
		*pixel_mode |= PMODE_GLOW;
	return 0;
}