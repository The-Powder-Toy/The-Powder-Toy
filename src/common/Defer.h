#pragma once
#include <functional>

class Defer
{
	std::function<void ()> func;

public:
	Defer(std::function<void ()> newFunc) : func(newFunc)
	{
	}

	Defer(const Defer &other) = delete;
	Defer &operator =(const Defer &other) = delete;

	~Defer()
	{
		if (func)
		{
			func();
		}
	}
};
