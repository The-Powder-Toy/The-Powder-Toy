#include <powder.h>

int update_MISC(UPDATE_FUNC_ARGS) { // should probably get their own functions later
	int t = parts[i].type;
	if ((t==PT_PTCT||t==PT_NTCT)&&parts[i].temp>295.0f)
	{
		parts[i].temp -= 2.5f;
	}
	if (t==PT_GLAS)
	{
		parts[i].pavg[0] = parts[i].pavg[1];
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
		if (parts[i].pavg[1]-parts[i].pavg[0] > 0.25f || parts[i].pavg[1]-parts[i].pavg[0] < -0.25f)
		{
			part_change_type(i,x,y,PT_BGLA);
		}
	}
	else if (t==PT_QRTZ)
	{
		parts[i].pavg[0] = parts[i].pavg[1];
		parts[i].pavg[1] = pv[y/CELL][x/CELL];
		if (parts[i].pavg[1]-parts[i].pavg[0] > 0.05*(parts[i].temp/3) || parts[i].pavg[1]-parts[i].pavg[0] < -0.05*(parts[i].temp/3))
		{
			part_change_type(i,x,y,PT_PQRT);
		}
	}
	else if (t==PT_PLUT && 1>rand()%100 && ((int)(5.0f*pv[y/CELL][x/CELL]))>(rand()%1000))
	{
		parts[i].type = PT_NEUT;
		create_part(i, x, y, parts[i].type);
	}
	else if (t==PT_MORT) {
		create_part(-1, x, y-1, PT_SMKE);
	}
	else if (t==PT_LOVE)
		ISLOVE=1;
	else if (t==PT_LOLZ)
		ISLOLZ=1;
	else if (t==PT_GRAV)
		ISGRAV=1;


	if (!legacy_enable) {
		if (t==PT_URAN && pv[y/CELL][x/CELL]>0.0f)
		{
			float atemp =  parts[i].temp + (-MIN_TEMP);
			parts[i].temp = restrict_flt((atemp*(1+(pv[y/CELL][x/CELL]/2000)))+MIN_TEMP, MIN_TEMP, MAX_TEMP);
		}
	}

	return 0;
}
