#pragma once
#include "common/Plane.h"
#include "SimulationConfig.h"
#include <cstdint>
#include <vector>

template<class Item>
using GravityPlane = PlaneAdapter<std::vector<Item>, CELLS.X, CELLS.Y>;

struct GravityInput
{
	GravityPlane<float> mass;
	GravityPlane<uint32_t> mask;
	static_assert(sizeof(float) == sizeof(uint32_t));

	GravityInput() : mass(CELLS, 0.f), mask(CELLS, UINT32_C(0xFFFFFFFF))
	{
	}
};

struct GravityOutput
{
	GravityPlane<float> forceX;
	GravityPlane<float> forceY;

	GravityOutput() : forceX(CELLS, 0.f), forceY(CELLS, 0.f)
	{
	}
};
