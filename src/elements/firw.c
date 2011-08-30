#include <element.h>

int update_FIRW(UPDATE_FUNC_ARGS) {
	int r, rx, ry, rt, np;
	if (parts[i].tmp==0) {
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = parts[r>>8].type;
					if (rt==PT_FIRE||rt==PT_PLSM||rt==PT_THDR)
					{
						parts[i].tmp = 1;
						parts[i].life = rand()%50+60;
					}
				}
	}
	else if (parts[i].tmp==1) {
		if (parts[i].life==0) {
			parts[i].tmp=2;
		} else {
			float newVel = parts[i].life/25;
			parts[i].flags = parts[i].flags&0xFFFFFFFE;
			/* TODO:
			if ((pmap[(int)(ly-newVel)][(int)lx]&0xFF)==PT_NONE && ly-newVel>0) {
				parts[i].vy = -newVel;
				ly-=newVel;
				iy-=newVel;
			}*/
			parts[i].vy = -newVel;
		}
	}
	else if (parts[i].tmp==2) {
		int col = rand()%200+4;
		int tmul;
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
				{
					tmul = rand()%7;
					np = create_part(-1, x+rx, y+ry, PT_FIRW);
					if (np>-1)
					{
						parts[np].vx = (rand()%3-1)*tmul;
						parts[np].vy = (rand()%3-1)*tmul;
						parts[np].tmp = col;
						parts[np].life = rand()%100+100;
						parts[np].temp = 6000.0f;
						parts[np].dcolour = parts[i].dcolour;
					}
				}
		pv[y/CELL][x/CELL] += 20;
		kill_part(i);
		return 1;
	} else if (parts[i].tmp>=3) {
		if (parts[i].life<=0) {
			kill_part(i);
			return 1;
		}
	}
	return 0;
}
