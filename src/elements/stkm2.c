#include <element.h>

int update_SPAWN2(UPDATE_FUNC_ARGS) {
	if (!player2[27])
		create_part(-1, x, y, PT_STKM2);

	return 0;
}

int update_STKM2(UPDATE_FUNC_ARGS) {
	run_stickman(player2, UPDATE_FUNC_SUBCALL_ARGS);
	return 0;
}

