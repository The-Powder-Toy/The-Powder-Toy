#pragma once
#include "GravityData.h"
#include "GravityPtr.h"

class Gravity
{
protected:
	Gravity() = default;

public:
	// potentially clobbers gravIn
	void Exchange(GravityOutput &gravOut, GravityInput &gravIn, bool forceRecalc);

	static GravityPtr Create();
};
