#include <element.h>

int update_GLAS(UPDATE_FUNC_ARGS) {
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = pv[y/CELL][x/CELL];
	if (parts[i].pavg[1]-parts[i].pavg[0] > 0.25f || parts[i].pavg[1]-parts[i].pavg[0] < -0.25f)
	{
		part_change_type(i,x,y,PT_BGLA);
	}
	return 0;
}
