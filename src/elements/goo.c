#include <powder.h>

int update_GOO(UPDATE_FUNC_ARGS) {
	if (!parts[i].life)
	{
		if (pv[y/CELL][x/CELL]>1.0f)
		{
			float advection = 0.1f;
			parts[i].vx += advection*vx[y/CELL][x/CELL];
			parts[i].vy += advection*vy[y/CELL][x/CELL];
			parts[i].life = rand()%80+300;
		}
	}
	return 0;
}
