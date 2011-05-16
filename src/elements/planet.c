#include <element.h>

int update_PLAN(UPDATE_FUNC_ARGS) {
	int r, rx, ry;
	if (ngrav_enable==1){
        gravmap[y/CELL][x/CELL] += parts[i].tmp;
    }
	return 0;
}
