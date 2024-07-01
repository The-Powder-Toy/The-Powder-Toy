#pragma once
#include "GravityData.h"
#include "GravityPtr.h"

class Gravity
{
protected:
	Gravity() = default;

public:
	void Exchange(GravityOutput &gravOut, const GravityInput &gravIn);

	static GravityPtr Create();
};
