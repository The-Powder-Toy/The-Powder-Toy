#include "simulation/Element.h"

int update_NBHL(UPDATE_FUNC_ARGS) {
	sim->gravmap[(y/CELL)*(XRES/CELL)+(x/CELL)] += 0.1f;
	return 0;
}
