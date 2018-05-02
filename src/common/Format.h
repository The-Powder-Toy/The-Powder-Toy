#pragma once

#include <functional>
#include <ios>

#include "common/String.h"

template<typename T> class FormatProxy
{
	T const &value;
	inline FormatProxy(T const &_value): value(_value) {}
public:
	inline void Write(StringBuilder &b) { b << value; }
};

class Format
{
	std::function<void(StringBuilder &)> writer;
public:
	template<typename T, typename... Ts> inline Format(T const &value, Ts... args):
		writer([value, args...](StringBuilder &b) { FormatProxy<T>(value, args...).Write(b); })
	{}

	friend StringBuilder &operator<<(StringBuilder &, Format const &);

	enum Base { Dec, Oct, Hex };
	enum Float { Default, Fixed, Scientific };
};

inline StringBuilder &operator<<(StringBuilder &b, Format const &f)
{
	f.writer(b);
	return b;
}

template<typename T> class IntegralFormatProxy
{
	T value;
	Format::Base base;
	size_t width;
public:
	inline IntegralFormatProxy(T _value, Format::Base _base = Format::Dec, size_t _width = 0): value(_value), base(_base), width(_width) {}
	inline void Write(StringBuilder &b)
	{
		std::ios_base::fmtflags oldflags = b.flags;
		b.flags &= ~std::ios_base::basefield;
		b.flags |= base == Format::Hex ? std::ios_base::hex : base == Format::Oct ? std::ios_base::oct : std::ios_base::dec;
		size_t oldwidth = b.width;
		b.width = width;
		b << value;
		b.flags = oldflags;
		b.width = oldwidth;
	}
};

template<> class FormatProxy<short int>: public IntegralFormatProxy<short int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<int>: public IntegralFormatProxy<int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<long int>: public IntegralFormatProxy<long int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<long long int>: public IntegralFormatProxy<long long int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<unsigned short int>: public IntegralFormatProxy<unsigned short int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<unsigned int>: public IntegralFormatProxy<unsigned int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<unsigned long int>: public IntegralFormatProxy<unsigned long int> { using IntegralFormatProxy::IntegralFormatProxy; };
template<> class FormatProxy<unsigned long long int>: public IntegralFormatProxy<unsigned long long int> { using IntegralFormatProxy::IntegralFormatProxy; };

template<typename T> class FloatingFormatProxy
{
	T value;
	size_t precision;
	Format::Float style;
	size_t width;
public:
	inline FloatingFormatProxy(T _value, size_t _precision, Format::Float _style = Format::Default, size_t _width = 0): value(_value), precision(_precision), style(_style), width(_width) {}
	inline void Write(StringBuilder &b)
	{
		std::ios_base::fmtflags oldflags = b.flags;
		b.flags &= ~std::ios_base::floatfield;
		b.flags |= style == Format::Fixed ? std::ios_base::fixed : style == Format::Scientific ? std::ios_base::scientific : std::ios_base::fmtflags();
		size_t oldwidth = b.width;
		b.width = width;
		size_t oldprecision = b.precision;
		b.precision = precision;
		b << value;
		b.flags = oldflags;
		b.width = oldwidth;
		b.precision = oldprecision;
	}
};

template<> class FormatProxy<float>: public FloatingFormatProxy<float> { using FloatingFormatProxy::FloatingFormatProxy; };
template<> class FormatProxy<double>: public FloatingFormatProxy<double> { using FloatingFormatProxy::FloatingFormatProxy; };
