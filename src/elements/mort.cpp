#include "simulation/Element.h"

int update_MORT(UPDATE_FUNC_ARGS) {
	sim->create_part(-1, x, y-1, PT_SMKE);
	return 0;
}
