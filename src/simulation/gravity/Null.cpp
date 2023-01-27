#include "Gravity.h"
#include "Misc.h"
#include <cstring>

// gravity without fast Fourier transforms

void Gravity::get_result()
{
	memcpy(&gravy[0], &th_gravy[0], NCELL * sizeof(float));
	memcpy(&gravx[0], &th_gravx[0], NCELL * sizeof(float));
	memcpy(&gravp[0], &th_gravp[0], NCELL * sizeof(float));
}

void Gravity::update_grav(void)
{
}

GravityPtr Gravity::Create()
{
	return GravityPtr(new Gravity(CtorTag{}));
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete ptr;
}
