#include <powder.h>

int update_FIRW(UPDATE_FUNC_ARGS) {
	int r, rt;
	if (parts[i].tmp==0) {
		for (nx=-1; nx<2; nx++)
			for (ny=-1; ny<2; ny++)
				if (x+nx>=0 && y+ny>0 &&
				        x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r)
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
			if ((pmap[(int)(ly-newVel)][(int)lx]&0xFF)==PT_NONE && ly-newVel>0) {
				parts[i].vy = -newVel;
				// TODO
				//ly-=newVel;
				//iy-=newVel;
			}
		}
	}
	else if (parts[i].tmp==2) {
		int col = rand()%200+4;
		for (nx=-2; nx<3; nx++) {
			for (ny=-2; ny<3; ny++) {
				if (x+nx>=0 && y+ny>0 && x+nx<XRES && y+ny<YRES && (nx || ny))
				{
					int tmul = rand()%7;
					create_part(-1, x+nx, y+ny, PT_FIRW);
					r = pmap[y+ny][x+nx];
					if ((r>>8)>=NPART || !r)
						continue;
					if (parts[r>>8].type==PT_FIRW) {
						parts[r>>8].vx = (rand()%3-1)*tmul;
						parts[r>>8].vy = (rand()%3-1)*tmul;
						parts[r>>8].tmp = col;
						parts[r>>8].life = rand()%100+100;
						parts[r>>8].temp = 6000.0f;
					}
				}
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
