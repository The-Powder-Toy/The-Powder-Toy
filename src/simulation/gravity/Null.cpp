#include "Gravity.h"

void Gravity::Exchange(GravityOutput &gravOut, GravityInput &gravIn, bool forceRecalc)
{
}

GravityPtr Gravity::Create()
{
	return GravityPtr(new Gravity());
}

void GravityDeleter::operator ()(Gravity *ptr) const
{
	delete ptr;
}
