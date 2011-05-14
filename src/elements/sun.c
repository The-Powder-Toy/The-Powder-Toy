#include <element.h>

int update_SUN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (ngrav_enable==1){
        gravmap[y/CELL][x/CELL] += 0.8f;
    }
    int orbd[4] = {0, 0, 0, 0};	//Orbital distances
		int orbl[4] = {0, 0, 0, 0};	//Orbital locations
		if (!parts[i].life) parts[i].life = rand();
		if (!parts[i].ctype) parts[i].ctype = rand();
		orbitalparts_get(parts[i].life, parts[i].ctype, orbd, orbl);
		for (r = 0; r < 4; r++) {
			if (orbd[r]>1) {
				orbd[r] -= 12;
				if (orbd[r]<1) {
					orbd[r] = (rand()%128)+128;
					orbl[r] = rand()%255;
				} else {
					orbl[r] += 2;
					orbl[r] = orbl[r]%255;
				}
			} else {
				orbd[r] = (rand()%128)+128;
				orbl[r] = rand()%255;
			}
		}
		orbitalparts_set(&parts[i].life, &parts[i].ctype, orbd, orbl);
	return 0;
}
