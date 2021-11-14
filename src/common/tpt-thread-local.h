#pragma once
#include "Config.h"

#ifdef __MINGW32__
# include <cstddef>

class ThreadLocalCommon
{
	ThreadLocalCommon(const ThreadLocalCommon &other) = delete;
	ThreadLocalCommon &operator =(const ThreadLocalCommon &other) = delete;

protected:
	size_t size;
	void (*ctor)(void *);
	void (*dtor)(void *);
	size_t padding;

	void *Get() const;

public:
	ThreadLocalCommon() = default;

	static constexpr size_t Alignment = 0x20;
};
// * If this fails, add or remove padding fields, possibly change Alignment to a larger power of 2.
static_assert(sizeof(ThreadLocalCommon) == ThreadLocalCommon::Alignment, "fix me");

template<class Type>
class ThreadLocal : public ThreadLocalCommon
{
	static void Ctor(void *type)
	{
		new(type) Type();
	}

	static void Dtor(void *type)
	{
		reinterpret_cast<Type *>(type)->~Type();
	}

public:
	ThreadLocal()
	{
		// * If this fails, you're out of luck.
		static_assert(sizeof(ThreadLocal<Type>) == sizeof(ThreadLocalCommon), "fix me");
		size = sizeof(Type);
		ctor = Ctor;
		dtor = Dtor;
	}

	Type *operator &() const
	{
		return reinterpret_cast<Type *>(Get());
	}

	operator Type &() const
	{
		return *(this->operator &());
	}
};

# define THREAD_LOCAL(Type, tl) const ThreadLocal<Type> tl __attribute__((section("tpt_tls"), aligned(ThreadLocalCommon::Alignment)))
#else
# define THREAD_LOCAL(Type, tl) thread_local Type tl
#endif
