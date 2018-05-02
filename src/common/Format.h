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

namespace Format
{
	template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> struct FlagsOverride
	{
		T value;
		inline FlagsOverride(T _value): value(_value) {}
	};
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> struct FlagsOverride<void, set, reset>
	{
		inline FlagsOverride() {}
	};

	template<typename T> struct WidthOverride
	{
		T value;
		size_t width;
		inline WidthOverride(T _value, size_t _width): value(_value), width(_width) {}
	};
	template<> struct WidthOverride<void>
	{
		size_t width;
		inline WidthOverride(size_t _width): width(_width) {}
	};

	template<typename T> struct PrecisionOverride
	{
		T value;
		size_t precision;
		inline PrecisionOverride(T _value, size_t _precision): value(_value), precision(_precision) {}
	};
	template<> struct PrecisionOverride<void>
	{
		size_t precision;
		inline PrecisionOverride(size_t _precision): precision(_precision) {}
	};

	template<typename T> inline FlagsOverride<T, std::ios_base::oct, std::ios_base::basefield> Oct(T value) { return FlagsOverride<T, std::ios_base::oct, std::ios_base::basefield>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::dec, std::ios_base::basefield> Dec(T value) { return FlagsOverride<T, std::ios_base::dec, std::ios_base::basefield>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::hex, std::ios_base::basefield> Hex(T value) { return FlagsOverride<T, std::ios_base::hex, std::ios_base::basefield>(value); }
	inline FlagsOverride<void, std::ios_base::oct, std::ios_base::basefield> Oct() { return FlagsOverride<void, std::ios_base::oct, std::ios_base::basefield>(); }
	inline FlagsOverride<void, std::ios_base::dec, std::ios_base::basefield> Dec() { return FlagsOverride<void, std::ios_base::dec, std::ios_base::basefield>(); }
	inline FlagsOverride<void, std::ios_base::hex, std::ios_base::basefield> Hex() { return FlagsOverride<void, std::ios_base::hex, std::ios_base::basefield>(); }

	template<typename T> inline FlagsOverride<T, std::ios_base::fixed, std::ios_base::floatfield> Fixed(T value) { return FlagsOverride<T, std::ios_base::fixed, std::ios_base::floatfield>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::scientific, std::ios_base::floatfield> Scientific(T value) { return FlagsOverride<T, std::ios_base::scientific, std::ios_base::floatfield>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::fmtflags{}, std::ios_base::floatfield> FloatDefault(T value) { return FlagsOverride<T, std::ios_base::fmtflags{}, std::ios_base::floatfield>(value); }
	inline FlagsOverride<void, std::ios_base::fixed, std::ios_base::floatfield> Fixed() { return FlagsOverride<void, std::ios_base::fixed, std::ios_base::floatfield>(); }
	inline FlagsOverride<void, std::ios_base::scientific, std::ios_base::floatfield> Scientific() { return FlagsOverride<void, std::ios_base::scientific, std::ios_base::floatfield>(); }
	inline FlagsOverride<void, std::ios_base::fmtflags{}, std::ios_base::floatfield> FloatDefault() { return FlagsOverride<void, std::ios_base::fmtflags{}, std::ios_base::floatfield>(); }

	template<typename T> inline WidthOverride<T> Width(T value, size_t width) { return WidthOverride<T>(value, width); }
	template<typename T> inline PrecisionOverride<T> Precision(T value, size_t precision) { return PrecisionOverride<T>(value, precision); }
	inline WidthOverride<void> Width(size_t width) { return WidthOverride<void>(width); }
	inline PrecisionOverride<void> Precision(size_t precision) { return PrecisionOverride<void>(precision); }
};

template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline StringBuilder &operator<<(StringBuilder &b, Format::FlagsOverride<T, set, reset> data)
{
	std::ios_base::fmtflags oldflags = b.flags;
	b.flags = (b.flags & ~reset) | set;
	b << data.value;
	b.flags = oldflags;
	return b;
}
template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline StringBuilder &operator<<(StringBuilder &b, Format::FlagsOverride<void, set, reset> data)
{
	b.flags = (b.flags & ~reset) | set;
	return b;
}

template<typename T> inline StringBuilder &operator<<(StringBuilder &b, Format::WidthOverride<T> data)
{
	size_t oldwidth = b.width;
	b.width = data.width;
	b << data.value;
	b.width = oldwidth;
	return b;
}
inline StringBuilder &operator<<(StringBuilder &b, Format::WidthOverride<void> data)
{
	b.width = data.width;
	return b;
}

template<typename T> inline StringBuilder &operator<<(StringBuilder &b, Format::PrecisionOverride<T> data)
{
	std::ios_base::fmtflags oldflags = b.flags;
	if(!(oldflags & std::ios_base::floatfield))
		b.flags |= std::ios_base::fixed;
	size_t oldprecision = b.precision;
	b.precision = data.precision;
	b << data.value;
	b.precision = oldprecision;
	b.flags = oldflags;
	return b;
}
inline StringBuilder &operator<<(StringBuilder &b, Format::PrecisionOverride<void> data)
{
	if(!(b.flags & std::ios_base::floatfield))
		b.flags |= std::ios_base::fixed;
	b.precision = data.precision;
	return b;
}
