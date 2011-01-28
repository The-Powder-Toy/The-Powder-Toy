#include <element.h>

int update_URAN(UPDATE_FUNC_ARGS) {
	if (!legacy_enable && pv[y/CELL][x/CELL]>0.0f)
	{
		float atemp =  parts[i].temp + (-MIN_TEMP);
		parts[i].temp = restrict_flt((atemp*(1+(pv[y/CELL][x/CELL]/2000)))+MIN_TEMP, MIN_TEMP, MAX_TEMP);
	}
	return 0;
}
