#include <element.h>

int update_ACLOUD(UPDATE_FUNC_ARGS) {
	int r,rx,ry;
    create_part(-1, x+rand()%3-1, y+rand()%5-1, PT_ACRN);
	return 0;
}
