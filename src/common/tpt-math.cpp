#include <cmath>
#include "TrigTables.h"

float orig_atan(float val)
{
	return atan(val);
}

namespace tpt
{

float sin(float angle)
{
	angle *= 81.4873;
	int i = (int)angle % 512;

	if (i < 0)
	{
		i += 512;
	}

	return sineLookupTable[i];
}


float cos(float angle)
{
	angle *= 81.4873;
	int i = (int)angle % 512;

	if (i < 0)
	{
		i += 512;
	}

	return cosineLookupTable[i];
}

float tan(float angle)
{
	angle *= 81.4873;
	int i = (int)angle % 512;
	if (i < 0)
	{
		i += 512;
	}

	return tanLookupTable[i];
}

float asin(float angle)
{
	angle *= 81.4873;
	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	return asinLookupTable[(int)(angle + 256)];
}

float acos(float angle)
{
	angle *= 81.4873;
	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	return acosLookupTable[(int)(angle + 256)];
}

float atan(float ratio)
{
	if (ratio > 20)
	{
		return orig_atan(ratio);
	}
	if (ratio < -20)
	{
		return orig_atan(ratio);
	}
	return atanLookupTable[(int)(ratio * 100) + 2000];
}


float atan2(float y, float x)
{
	if (x > 0)
	{
		return tpt::atan(y / x);
	}
	else if (x < 0)
	{
		if (y >= 0)
		{
			return tpt::atan(y / x) + M_PI;
		}

		return tpt::atan(y / x) - M_PI;
	}
	else if (y > 0)
		return M_PI_2;
	else if (y < 0)
		return M_PI_2;
	else
		return 0.0f;
}

}
