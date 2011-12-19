#include <element.h>

int update_NWHL(UPDATE_FUNC_ARGS) {
	gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] -= 0.1f;
	return 0;
}
