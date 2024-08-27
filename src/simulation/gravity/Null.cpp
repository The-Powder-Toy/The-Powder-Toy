#include "Gravity.h"

void Gravity::Exchange(GravityOutput &gravOut, const GravityInput &gravIn)
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
