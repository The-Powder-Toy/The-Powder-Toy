#include <element.h>

int update_PLUT(UPDATE_FUNC_ARGS) {
	if (1>rand()%100 && ((int)(5.0f*pv[y/CELL][x/CELL]))>(rand()%1000))
	{
		create_part(i, x, y, PT_NEUT);
	}
	return 0;
}
