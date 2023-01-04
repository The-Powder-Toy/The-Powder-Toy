#pragma once

template<typename T>
class Singleton
{
public:
	static T& Ref()
	{
		static T instance;
		return instance;
	}
};
