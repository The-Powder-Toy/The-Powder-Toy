#include <element.h>

int update_NWHL(UPDATE_FUNC_ARGS) {
    if(ngrav_enable)
        gravmap[y/CELL][x/CELL] -= 0.1f;
	return 0;
}
