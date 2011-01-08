#include <powder.h>

int update_PHOT(UPDATE_FUNC_ARGS) {
	if (1>rand()%10) update_PYRO(UPDATE_FUNC_SUBCALL_ARGS);
	// TODO

	return 0;
}
