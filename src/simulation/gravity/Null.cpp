#include "Gravity.h"
#include "Misc.h"
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
}

GravityPtr Gravity::Create()
{
	return GravityPtr(new Gravity(CtorTag{}));
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete ptr;
}
