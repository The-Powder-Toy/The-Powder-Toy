#include <element.h>

int update_GENR(UPDATE_FUNC_ARGS) {
    int r,rx,ry;
    if (pv[y/CELL][x/CELL] > 4.0f){
        parts[i].ctype=PT_NCGN;
                    parts[i].life = 4;
                    parts[i].type = PT_SPRK;
                    parts[r>>PS].type = PT_NCWS;
    }
	return 0;
}
