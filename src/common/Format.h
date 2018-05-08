#pragma once

#include <functional>
#include <ios>

#include "common/String.h"

/*
	The following formatting specifiers can be written into a
	ByteStringBuilder or a StringBuilder:

	Format::Oct(value)
	Format::Dec(value)
	Format::Hex(value)
		Write the value in the specified base.

	Format::Uppercase(value)
	Format::NoUppercase(value)
		Toggle uppercase characters in hexadecimal and scientific form.

	Format::ShowPoint(value)
	Format::NoShowPoint(value)
		In floats, toggle always displaying the decimal point even if
		the number is an integer.

	Format::Fixed(value)
	Format::Scientific(value)
		Display the float with a fixed number of digits after the
		decimal point, or force using the scientific notation.

	Format::Precision(value, size_t precision)
		Fix the number of digits of precision used for floats. By
		default also enables the Fixed mode.

	Format::Width(value, size_t width)
		Fix the number of characters used to represent the value. By
		default also sets the fill to the digit '0'.

	All of the above can be written into builders with the value argument
	omitted. In that case the specifiers will affect all future writes to
	the builder.

	The following formatting specifiers can be passed to string functions
	SplitNumber and ToNumber:

	Format::Oct()
	Format::Dec()
	Format::Hex()
		Read the value in the specified base.

	Format::SkipWS()
	Format::NoSkipWS()
		Toggle ignoring the whitespace when reading a number.
*/

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

	template<typename T> struct FillOverride
	{
		T value;
		size_t fill;
		inline FillOverride(T _value, size_t _fill): value(_value), fill(_fill) {}
	};
	template<> struct FillOverride<void>
	{
		String::value_type fill;
		inline FillOverride(size_t _fill): fill(_fill) {}
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

	template<typename T> inline FlagsOverride<T, std::ios_base::uppercase, std::ios_base::uppercase> Uppercase(T value) { return FlagsOverride<T, std::ios_base::uppercase, std::ios_base::uppercase>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::showpoint, std::ios_base::showpoint> ShowPoint(T value) { return FlagsOverride<T, std::ios_base::showpoint, std::ios_base::showpoint>(value); }
	template<typename T> inline FlagsOverride<T, EmptyFmtFlags, std::ios_base::uppercase> NoUppercase(T value) { return FlagsOverride<T, EmptyFmtFlags, std::ios_base::uppercase>(value); }
	template<typename T> inline FlagsOverride<T, EmptyFmtFlags, std::ios_base::showpoint> NoShowPoint(T value) { return FlagsOverride<T, EmptyFmtFlags, std::ios_base::showpoint>(value); }
	inline FlagsOverride<void, std::ios_base::uppercase, std::ios_base::uppercase> Uppercase() { return FlagsOverride<void, std::ios_base::uppercase, std::ios_base::uppercase>(); }
	inline FlagsOverride<void, std::ios_base::showpoint, std::ios_base::showpoint> ShowPoint() { return FlagsOverride<void, std::ios_base::showpoint, std::ios_base::showpoint>(); }
	inline FlagsOverride<void, std::ios_base::skipws, std::ios_base::skipws> SkipWS() { return FlagsOverride<void, std::ios_base::skipws, std::ios_base::skipws>(); }
	inline FlagsOverride<void, EmptyFmtFlags, std::ios_base::uppercase> NoUppercase() { return FlagsOverride<void, EmptyFmtFlags, std::ios_base::uppercase>(); }
	inline FlagsOverride<void, EmptyFmtFlags, std::ios_base::showpoint> NoShowPoint() { return FlagsOverride<void, EmptyFmtFlags, std::ios_base::showpoint>(); }
	inline FlagsOverride<void, EmptyFmtFlags, std::ios_base::skipws> NoSkipWS() { return FlagsOverride<void, EmptyFmtFlags, std::ios_base::skipws>(); }

	template<typename T> inline FlagsOverride<T, std::ios_base::fixed, std::ios_base::floatfield> Fixed(T value) { return FlagsOverride<T, std::ios_base::fixed, std::ios_base::floatfield>(value); }
	template<typename T> inline FlagsOverride<T, std::ios_base::scientific, std::ios_base::floatfield> Scientific(T value) { return FlagsOverride<T, std::ios_base::scientific, std::ios_base::floatfield>(value); }
	template<typename T> inline FlagsOverride<T, EmptyFmtFlags, std::ios_base::floatfield> FloatDefault(T value) { return FlagsOverride<T, EmptyFmtFlags, std::ios_base::floatfield>(value); }
	inline FlagsOverride<void, std::ios_base::fixed, std::ios_base::floatfield> Fixed() { return FlagsOverride<void, std::ios_base::fixed, std::ios_base::floatfield>(); }
	inline FlagsOverride<void, std::ios_base::scientific, std::ios_base::floatfield> Scientific() { return FlagsOverride<void, std::ios_base::scientific, std::ios_base::floatfield>(); }
	inline FlagsOverride<void, EmptyFmtFlags, std::ios_base::floatfield> FloatDefault() { return FlagsOverride<void, EmptyFmtFlags, std::ios_base::floatfield>(); }

	template<typename T> inline FillOverride<T> Fill(T value, String::value_type fill) { return FillOverride<T>(value, fill); }
	template<typename T> inline WidthOverride<T> Width(T value, size_t width) { return WidthOverride<T>(value, width); }
	template<typename T> inline PrecisionOverride<T> Precision(T value, size_t precision) { return PrecisionOverride<T>(value, precision); }
	inline FillOverride<void> Fill(String::value_type fill) { return FillOverride<void>(fill); }
	inline WidthOverride<void> Width(size_t width) { return WidthOverride<void>(width); }
	inline PrecisionOverride<void> Precision(size_t precision) { return PrecisionOverride<void>(precision); }
};

template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::FlagsOverride<T, set, reset> data)
{
	std::ios_base::fmtflags oldflags = b.flags;
	b.flags = (b.flags & ~reset) | set;
	b << data.value;
	b.flags = oldflags;
	return b;
}
template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::FlagsOverride<void, set, reset> data)
{
	b.flags = (b.flags & ~reset) | set;
	return b;
}

template<typename T> inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::FillOverride<T> data)
{
	size_t oldfill = b.fill;
	b.fill = data.fill;
	b << data.value;
	b.fill = oldfill;
	return b;
}
inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::FillOverride<void> data)
{
	b.fill = data.fill;
	return b;
}

template<typename T> inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::WidthOverride<T> data)
{
	String::value_type oldfill = b.fill;
	if(oldfill == ' ')
		b.fill = '0';
	size_t oldwidth = b.width;
	b.width = data.width;
	b << data.value;
	b.width = oldwidth;
	b.fill = oldfill;
	return b;
}
inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::WidthOverride<void> data)
{
	if(b.fill == ' ')
		b.fill = '0';
	b.width = data.width;
	return b;
}

template<typename T> inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::PrecisionOverride<T> data)
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
inline ByteStringBuilder &operator<<(ByteStringBuilder &b, Format::PrecisionOverride<void> data)
{
	if(!(b.flags & std::ios_base::floatfield))
		b.flags |= std::ios_base::fixed;
	b.precision = data.precision;
	return b;
}

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

template<typename T> inline StringBuilder &operator<<(StringBuilder &b, Format::FillOverride<T> data)
{
	size_t oldfill = b.fill;
	b.fill = data.fill;
	b << data.value;
	b.fill = oldfill;
	return b;
}
inline StringBuilder &operator<<(StringBuilder &b, Format::FillOverride<void> data)
{
	b.fill = data.fill;
	return b;
}

template<typename T> inline StringBuilder &operator<<(StringBuilder &b, Format::WidthOverride<T> data)
{
	String::value_type oldfill = b.fill;
	if(oldfill == ' ')
		b.fill = '0';
	size_t oldwidth = b.width;
	b.width = data.width;
	b << data.value;
	b.width = oldwidth;
	b.fill = oldfill;
	return b;
}
inline StringBuilder &operator<<(StringBuilder &b, Format::WidthOverride<void> data)
{
	if(b.fill == ' ')
		b.fill = '0';
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
