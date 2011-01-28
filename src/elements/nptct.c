#include <element.h>

int update_NPTCT(UPDATE_FUNC_ARGS) {
	if (parts[i].temp>295.0f)
		parts[i].temp -= 2.5f;
	return 0;
}
