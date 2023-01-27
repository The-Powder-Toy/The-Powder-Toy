#pragma once
#include <memory>

class Gravity;
struct GravityDeleter
{
	void operator ()(Gravity *ptr) const;
};
using GravityPtr = std::unique_ptr<Gravity, GravityDeleter>;
