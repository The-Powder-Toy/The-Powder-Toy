#include <element.h>

int update_VINE(UPDATE_FUNC_ARGS) {
	int r, np, rx =(rand()%3)-1, ry=(rand()%3)-1;
	if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
	{
		r = pmap[y+ry][x+rx];
		if (1>rand()%15)
			part_change_type(i,x,y,PT_PLNT);
		else if (!r)
		{
			np = create_part(-1,x+rx,y+ry,PT_VINE);
			if (np<0) return 0;
			parts[np].temp = parts[i].temp;
			part_change_type(i,x,y,PT_PLNT);
		}
	}
	return 0;
}
