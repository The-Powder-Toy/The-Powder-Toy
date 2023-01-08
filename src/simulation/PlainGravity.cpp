#include "Gravity.h"
#include "Misc.h"
#include <cmath>
#include <cstring>

// gravity without fast Fourier transforms

void Gravity::get_result()
{
	memcpy(gravy, th_gravy, NCELL*sizeof(float));
	memcpy(gravx, th_gravx, NCELL*sizeof(float));
	memcpy(gravp, th_gravp, NCELL*sizeof(float));
}

void Gravity::update_grav(void)
{
	th_gravchanged = 1;
	membwand(th_gravmap, gravmask, NCELL*sizeof(float), NCELL*sizeof(unsigned));
	for (int i = 0; i < YCELLS; i++)
	{
		for (int j = 0; j < XCELLS; j++)
		{
			if (th_ogravmap[i*XCELLS+j] != th_gravmap[i*XCELLS+j])
			{
				for (int y = 0; y < YCELLS; y++)
				{
					for (int x = 0; x < XCELLS; x++)
					{
						if (x == j && y == i)//Ensure it doesn't calculate with itself
							continue;
						auto distance = hypotf(j - x, i - y);
						float val;
						val = th_gravmap[i*XCELLS+j] - th_ogravmap[i*XCELLS+j];
						th_gravx[y*XCELLS+x] += M_GRAV * val * (j - x) / powf(distance, 3.0f);
						th_gravy[y*XCELLS+x] += M_GRAV * val * (i - y) / powf(distance, 3.0f);
						th_gravp[y*XCELLS+x] += M_GRAV * val / powf(distance, 2.0f);
					}
				}
			}
		}
	}
	memcpy(th_ogravmap, th_gravmap, NCELL*sizeof(float));
}

GravityPtr Gravity::Create()
{
	return GravityPtr(new Gravity(CtorTag{}));
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete ptr;
}
