#include <element.h>

int update_CLOUD(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
    create_part(-1, x+rand()%3-1, y+rand()%5-1, PT_RWTR);
	return 0;
}
