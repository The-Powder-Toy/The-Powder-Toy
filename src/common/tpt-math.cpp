namespace tpt
{
#include "TrigTables.h"
float sin(float angle)
	{
		float sine;
		int i;
		
		angle *= 81.4873;
		i = (int)angle % 512;
		if (i < 0)
		{
			i += 512;
		}

		sine = sineLookupTable[i];

		return sine;
	}


float cos(float angle)
	{
		float cosine;
		int i;

		angle *= 81.4873;
		i = (int)angle % 512;
		if (i < 0)
		{
			i += 512;
		}

		cosine = cosineLookupTable[i];

		return cosine;
	}

float tan(float angle)
	{
		float tangent;
		int i;

		angle *= 81.4873;
		i = (int)angle % 512;
		if (i < 0)
		{
			i += 512;
		}

		tangent = tanLookupTable[i];

		return tangent;
	}

float asin(float angle)
{
	float asin;

	angle *= 81.4873;
	
	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	asin = asinLookupTable[(int)(angle + 256)];

	return asin;
}

float acos(float angle)
{
	float acos;

	angle *= 81.4873;

	if (angle > 256 || angle < 0)
	{
		return 0.0;
	}

	acos = acosLookupTable[(int)(angle + 256)];

	return acos;
}

float atan(float ratio)
	{
		float atan;

		if (ratio > 20)
		{
			return 1.53f;
		}
		if (ratio < -20)
		{
			return -1.53f;
		}
		atan = atanLookupTable[(signed int)(ratio * 100) + 2000];
		return atan;
	}


float atan2(float y, float x)
	{
		float atan2, debugtan;

		if (x > 0)
		{
			atan2 = atan((y / x));
			return atan2;
		}

		else
		{
			if (y >= 0)
			{
				atan2 = atan((y / x)) + 3.14159f;
				return atan2;
			}

			atan2 = atan((y / x)) - 3.14159f;
			return atan2;
	}

}











}