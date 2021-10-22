#pragma once
#include "Config.h"

#ifdef __MINGW32__
# include <cstddef>

template<class Type>
class ThreadLocal
{
	static void Ctor(Type *type)
	{
		new (type) Type();
	}

	static void Dtor(Type *type)
	{
		type->~Type();
	}

	size_t size = sizeof(Type);
	void (*ctor)(Type *) = Ctor;
	void (*dtor)(Type *) = Dtor;
	size_t padding;

public:
	Type *operator &()
	{
		static_assert(sizeof(ThreadLocal<Type>) == 0x20, "fix me");
		void *ThreadLocalGet(void *opaque);
		return reinterpret_cast<Type *>(ThreadLocalGet(reinterpret_cast<void *>(this)));
	}

	operator Type &()
	{
		return *(this->operator &());
	}
};

# define THREAD_LOCAL(Type, tl) ThreadLocal<Type> tl __attribute__((section("tpt_tls"))) __attribute__((aligned(0x20)))
#else
# define THREAD_LOCAL(Type, tl) thread_local Type tl
#endif
