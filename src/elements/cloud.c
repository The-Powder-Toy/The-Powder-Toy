#include <element.h>

int update_CLOUD(UPDATE_FUNC_ARGS) {
	int r,rx,ry,tmp = parts[i].tmp;
	int self = parts[i].type;
    if (rand()%100 == 1){
        if (parts[i].tmp = 1){
            parts[i].tmp = 0;
        }
    }
    if (rand()%1000 == 1 && rand()%700 == 1 && parts[i].tmp2 == 2){
        int pt1;
        int pt2;
        int num = YRES/4;
        if (rand()%2 == 1){
            pt1 = rand()%16;
            pt2 = rand()%32;
            create_line(x,       y,         x - pt1, y + num*1, 0, 0, PT_LTNG);
            create_line(x - pt1, y + num,   x + pt2, y + num*2, 0, 0, PT_LTNG);
            create_line(x + pt2, y + num*2, x - pt1, y + num*3, 0, 0, PT_LTNG);
            create_line(x - pt1, y + num*3, x + pt2, y + num*4, 0, 0, PT_LTNG);
        } else {
            pt1 = rand()%32;
            pt2 = rand()%16;
            create_line(x,       y,         x + pt1, y + num*1, 0, 0, PT_LTNG);
            create_line(x + pt1, y + num*1, x - pt2, y + num*2, 0, 0, PT_LTNG);
            create_line(x - pt2, y + num*2, x + pt1, y + num*3, 0, 0, PT_LTNG);
            create_line(x + pt1, y + num*3, x - pt2, y + num*4, 0, 0, PT_LTNG);
        }



    }
    for (rx=-8; rx<12; rx++)
		for (ry=-8; ry<12; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
                if (parts[r>>PS].type == PT_RWTR){
                    if (rand()%1000 == 1){
                        if (!(parts[r>>PS].y < parts[i].y+2)){
                            parts[r>>PS].type=self;
                        }
                    }
                }
                if (parts[r>>PS].type == self){
                    if (parts[i].y >= parts[r>>PS].y){
                        if (parts[i].tmp != 1){
                            if (rand()%10 == 1){
                                create_part(-1, x+rand()%3-1, y+rand()%6+1, PT_RWTR);
                            }
                        }
                    } else {
                        parts[i].tmp = 1;
                    }
                }

            }
	return 0;
}

int update_ACLOUD(UPDATE_FUNC_ARGS) {
	int r,rx,ry,tmp = parts[i].tmp;
	int self = parts[i].type;
    if (rand()%100 == 1){
        if (parts[i].tmp = 1){
            parts[i].tmp = 0;
        }
    }
    for (rx=-8; rx<12; rx++)
		for (ry=-8; ry<12; ry++)
			if (x+rx>=0 && y+ry>0 && x+rx<XRES && y+ry<YRES && (rx || ry))
			{
				r = pmap[y+ry][x+rx];
				if ((r>>PS)>=NPART || !r)
					continue;
                if (parts[r>>PS].type == PT_ACRN){
                    if (rand()%1000 == 1){
                        if (parts[r>>PS].y > parts[i].y+2){
                            parts[r>>PS].type=self;
                        }
                    }
                }
                if (parts[r>>PS].type == self){
                    if (parts[i].y >= parts[r>>PS].y){
                        if (parts[i].tmp != 1){
                            if (rand()%10 == 1){
                            create_part(-1, x+rand()%3-1, y+rand()%6+1, PT_ACRN);
                            }
                        }
                } else {
                    parts[i].tmp = 1;
                }
                }

            }
	return 0;
}

