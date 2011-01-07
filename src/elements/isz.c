#include <powder.h>

int update_ISZ(UPDATE_FUNC_ARGS) {
	float rr, rrr;
	if (1>rand()%200 && ((int)(-4.0f*(pv[y/CELL][x/CELL])))>(rand()%1000))
			{
				parts[i].type = PT_PHOT;
				rr = (rand()%228+128)/127.0f;
				rrr = (rand()%360)*3.14159f/180.0f;
				parts[i].life = 680;
				parts[i].ctype = 0x3FFFFFFF;
				parts[i].vx = rr*cosf(rrr);
				parts[i].vy = rr*sinf(rrr);
				create_part(i, x, y, PT_PHOT);
			}
	return 0;
}
