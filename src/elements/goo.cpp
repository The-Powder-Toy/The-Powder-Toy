#include "simulation/Element.h"

int update_GOO(UPDATE_FUNC_ARGS) {
	if (!parts[i].life && sim->pv[y/CELL][x/CELL]>1.0f)
		parts[i].life = rand()%80+300;
	if (parts[i].life)
	{
		float advection = 0.1f;
		parts[i].vx += advection*sim->vx[y/CELL][x/CELL];
		parts[i].vy += advection*sim->vy[y/CELL][x/CELL];
	}
	return 0;
}
