#ifndef SINGLETON_H
#define SINGLETON_H

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

#endif // SINGLETON_H
