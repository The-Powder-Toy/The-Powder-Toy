#include <element.h>

int update_ISZ(UPDATE_FUNC_ARGS) { // for both ISZS and ISOZ
	float rr, rrr;
	if (1>rand()%200 && ((int)(-4.0f*(pv[y/CELL][x/CELL])))>(rand()%1000))
	{
		create_part(i, x, y, PT_PHOT);
		rr = (rand()%228+128)/127.0f;
		rrr = (rand()%360)*3.14159f/180.0f;
		parts[i].vx = rr*cosf(rrr);
		parts[i].vy = rr*sinf(rrr);
	}
	return 0;
}
