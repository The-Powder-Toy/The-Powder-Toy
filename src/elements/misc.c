#include <element.h>

int update_MISC(UPDATE_FUNC_ARGS) {
	int t = parts[i].type;
	if (t==PT_LOVE)
		ISLOVE=1;
	else if (t==PT_LOLZ)
		ISLOLZ=1;
	else if (t==PT_GRAV)
		ISGRAV=1;
	return 0;
}
