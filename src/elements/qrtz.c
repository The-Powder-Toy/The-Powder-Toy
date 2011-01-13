#include <powder.h>

int update_QRTZ(UPDATE_FUNC_ARGS) {
	parts[i].pavg[0] = parts[i].pavg[1];
	parts[i].pavg[1] = pv[y/CELL][x/CELL];
	if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
	{
		part_change_type(i,x,y,PT_PQRT);
	}
	return 0;
}
