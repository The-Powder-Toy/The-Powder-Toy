#include <powder.h>

int update_VINE(UPDATE_FUNC_ARGS) {
	int r;
	nx=(rand()%3)-1;
	ny=(rand()%3)-1;
	if (x+nx>=0 && y+ny>0 &&
	        x+nx<XRES && y+ny<YRES && (nx || ny))
	{
		r = pmap[y+ny][x+nx];
		if ((r>>8)>=NPART)
			return 1;
		if (1>rand()%15)
			parts[i].type=PT_PLNT;
		else if (!r)
		{
			create_part(-1,x+nx,y+ny,PT_VINE);
			parts[pmap[y+ny][x+nx]>>8].temp = parts[i].temp;
			parts[i].type=PT_PLNT;
		}
	}
	return 0;
}
