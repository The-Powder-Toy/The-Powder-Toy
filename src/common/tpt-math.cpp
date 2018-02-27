#include "TrigTables.h"

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

	float sine = sineLookupTable[i];
	return sine;
}


float cos(float angle)
{
	angle *= 81.4873;
	int i = (int)angle % 512;

	if (i < 0)
	{
		i += 512;
	}

	float cosine = cosineLookupTable[i];
	return cosine;
}

float tan(float angle)
{
	angle *= 81.4873;
	int i = (int)angle % 512;
	if (i < 0)
	{
		i += 512;
	}

	float tangent = tanLookupTable[i];
	return tangent;
}

float asin(float angle)
{
	angle *= 81.4873;
	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	float asin = asinLookupTable[(int)(angle + 256)];
	return asin;
}

float acos(float angle)
{
	angle *= 81.4873;
	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	float acos = acosLookupTable[(int)(angle + 256)];
	return acos;
}

float atan(float ratio)
{
	if (ratio > 20)
	{
		atan(ratio);
	}
	if (ratio < -20)
	{
		atan(ratio);
	}
	float atan = atanLookupTable[(int)(ratio * 100) + 2000];
	return atan;
}


float atan2(float y, float x)
{
	float atan2;

	if (x > 0)
	{
		atan2 = atan(y / x);
		return atan2;
	}
	else
	{
		if (y >= 0)
		{
			atan2 = atan(y / x) + 3.14159f;
			return atan2;
		}

		atan2 = atan(y / x) - 3.14159f;
		return atan2;
	}

}

}
