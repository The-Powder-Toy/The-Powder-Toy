#pragma once

#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <locale>
#include <ios>

/*
	There are two "string" classes: ByteString and String. They have nearly
	identical interfaces, except that one stores 8-bit octets (bytes) and
	the other stores Unicode codepoints.

	For ease of use, a String can be constructed from a string literal
	(char const[]) however not char const *. Exercise care when working
	with char arrays.

	Both classes inherit from std::basic_string (std::string is an
	instatiation of that), so all the familiar string interface is there
	however some helper methods have been defined:

	Substr(size_t start = 0, size_t count = npos)
		Returns a substring starting at position <start> and counting
		<count> symbols, or until end of string, whichever is earlier.
		If count == npos, the entire remainder of the string is
		included.

	SubstrFromEnd(size_t rstart = 0, size_t rcount = npos)
		Behaviourally equal to
			reverse(reverse(str).Substr(rstart, rcount))
		but is more efficient. Useful for taking suffixes of given
		length or dropping a fixed number of symbols from the end.

	Between(size_t begin, size_t end)
		Returns a substring starting at <begin> and ending at <end>.
		If end == npos, length of the string is used. If begin > end,
		an empty string is returned.

	Insert(size_t pos, String str)
		Inserts the characters from <str> at position <pos> shifting
		the rest of the string to the right.

	Erase(size_t pos, size_t count)
		Starting at position <pos> erases <count> characters to the
		right or until the end of string. The rest of the string is
		shifted left to fill the gap.

	EraseBetween(size_t from, size_t to)
		Starting at position <pos> erase until position <to> or end of
		string, whichever is earlier. The rest of the string is shifted
		left to fill the gap.

	BeginsWith(String prefix)
	EndsWith(String suffix)
	Contains(String infix)
	Contains(value_type infix)
		Self-explanatory.

	ToLower()
	ToUpper()
		Lowercases/Uppercases characters in the string. Only works on
		characters in the ASCII range.

	ByteString::FromUtf8(bool ignoreError = true)
		Decodes UTF-8 byte sequences into Unicode codepoints.
		If ignoreError is true, then invalid byte sequences are widened
		as-is. Otherwise, a ConversionError is thrown.

	ByteString::FromAscii()
		Interprets byte values as Unicode codepoints.

	String::ToUtf8()
		Encodes Unicode codepoints into UTF-8 byte sequences.

	String::ToAscii()
		Narrows Unicode codepoints into bytes, possibly truncating
		them (!).

	To convert something into a string use ByteStringBuilder and
	StringBuilder respectively. The two use operator<< much like
	std::ostringstream. To convert a builder to a string use the Build()
	method. Alternatively you could use the static ByteString::Build and
	String::Build methods respectively. String::Build(x, y, z) is roughly
	equivalent to:
		StringBuilder tmp;
		tmp << x << y << z;
		return tmp.Build();

	To control formatting of the input/output see "common/Format.h".

	If you simply want to convert a string to a number you can use the
	ToNumber<type>(bool ignoreError = false) method. It can optionally take
	a formatting specifier as an argument:
		str.ToNumber<unsigned>(Format::Hex(), true)

	Otherwise to parse a string into components you can use splits. A Split
	is a temporary object that "remembers" how a string is divided into a
	"prefix", a "separator", and a "suffix". A split can also "fail", if,
	for example the separator was not found in the string. A Split has the
	following methods:

	Before(bool includeSeparator = false)
		Returns the "prefix", optionally with the "separator".

	After(bool includeSeparator = false)
		Returns the "suffix", optionally with the "separator".

	A Split can also be converted to bool (used in the condition of an 'if'
	or a 'while'), in which case it shows whether the split had succeeded
	or failed. The idiomatic code goes like:
		if(String::Split split = str.SplitBy(','))
			// use split.Before() and split.After()
		else
			// str does not contain a ','

	The following methods split a string:

	SplitBy(String sep, size_t from = 0)
	SplitBy(value_type sep, size_t from = 0)
		Split on the first occurence of the <sep> separator since
		position <from>. If no such separator is found the split fails
		and "prefix" contains the whole string starting from <from>.

	SplitByAny(String chars, size_t from = 0)
		Split on the first occurence of any of the characters in
		<chars>.

	SplitByNot(String chars, size_t from = 0)
		Split on the first occurence of any character that is *not* in
		<chars>.

	SplitFromEndBy(String sep, size_t from = npos)
	SplitFromEndBy(value_type sep, size_t from = npos)
	SplitFromEndByAny(String chars, size_t from = npos)
	SplitFromEndByNot(String chars, size_t from = npos)
		These do the same as the functions above except they try to
		find the *last* occurence of the separator. If the split fails
		it is the "suffix" that contains the whole string, and the
		"prefix" is empty instead.

	SplitNumber(number &ref, size_t pos = 0)
		Attempt to read a number (with the type indicated by the type
		of the reference) from position <pos>. In case of success store
		the parsed number in the reference and return a split at the
		end of the parse. The separator in this case is empty. If the
		parse fails, the "prefix" is empty.

	SplitNumber(number &ref, format, size_t pos = 0)
		Parse the number according to the provided formatting
		specifier.

	It is recommented to use ByteString::value_type and String::value_type
	instead of char and char32_t respectively.

	The reason we do not use std::wstring is that on Windows, wchar_t is a
	16-bit type, which forces the usage of a UTF-16 (UCS-2) encoding to
	store the higher parts of the Unicode. The std::wstring implementation
	does not care to handle the UTF-16 encoding. Considering characters
	still occupy a variable amount of space (1 or 2 wchar_t's), finding the
	"n'th character" or "index of character at offset n" becomes a problem.
	(Event if that were not a problem a better solution would be to use the
	more space-efficient UTF-8). Therefore the String class is derived from
	std::basic_string<char32_t>, where char32_t is a type that is
	guaranteed to contain at least 32 bits. The drawback is that we
	basically lose std::stringstream (std::basic_stringstream) support and
	have to implement our own (See "common/String.cpp").
*/

class ByteStringBuilder;
class String;
class StringBuilder;
namespace Format
{
	const std::ios_base::fmtflags EmptyFmtFlags = std::ios_base::dec & ~std::ios_base::dec;
	template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> struct FlagsOverride;
}

template<typename T> class SplitBase
{
	T const &parent;
	size_t posFrom;
	size_t posBefore;
	size_t posAfter;
	bool reverse;

	inline SplitBase(T const &_parent, size_t _posFrom, size_t _posBefore, size_t offset, bool _reverse):
		parent(_parent),
		posFrom(_posFrom),
		posBefore(_posBefore),
		posAfter(_posBefore == T::npos ? T::npos : _posBefore + offset),
		reverse(_reverse)
	{}
public:
	inline T Before(bool includeSeparator = false) const
	{
		if(posBefore == T::npos)
			return reverse ? T() : parent.Substr(posFrom);
		return parent.Between(reverse ? 0 : posFrom, includeSeparator ? posAfter : posBefore);
	}
	inline T After(bool includeSeparator = false) const
	{
		if(posBefore == T::npos)
			return reverse ? parent.Substr(0, posFrom) : T();
		return parent.Between(includeSeparator ? posBefore : posAfter, reverse ? posFrom : T::npos);
	}

	inline size_t PositionFrom() const { return posFrom; }
	inline size_t PositionBefore() const { return posBefore; }
	inline size_t PositionAfter() const { return posAfter; }

	inline operator bool() const { return posBefore != T::npos; }

	friend T;
};

class ByteString : public std::basic_string<char>
{
	using super = std::basic_string<char>;
public:
	inline ByteString(): super() {}
	inline ByteString(value_type ch): super(1, ch) {}
	inline ByteString(size_type count, value_type ch): super(count, ch) {}
	inline ByteString(value_type const *ch, size_type count): super(ch, count) {}
	inline ByteString(value_type const *ch): super(ch) {}
	template<class It> inline ByteString(It first, It last): super(first, last) {}
	inline ByteString(super const &other): super(other) {}
	inline ByteString(super &&other): super(std::move(other)) {}
	inline ByteString(ByteString const &other): super(other) {}
	inline ByteString(ByteString &&other): super(std::move(other)) {}

	inline ByteString &operator=(ByteString const &other) { super::operator=(other); return *this; }
	inline ByteString &operator=(ByteString &&other) { super::operator=(other); return *this; }

	inline ByteString &operator+=(ByteString const &other) { super::operator+=(other); return *this; }
	inline ByteString &operator+=(ByteString &&other) { super::operator+=(std::move(other)); return *this; }

	inline ByteString Substr(size_t pos = 0, size_t count = npos) const { return super::substr(pos, count); }
	inline ByteString SubstrFromEnd(size_t rpos = 0, size_t rcount = npos) const { return super::substr(rcount == npos || rcount > rpos ? 0 : rpos - rcount, size() - rpos); }
	inline ByteString Between(size_t from, size_t to) const { return to == npos ? super::substr(from) : from >= to ? ByteString() : super::substr(from, to - from); }

	inline bool Contains(value_type ch) const { return super::find(ch) != npos; }
	inline bool Contains(ByteString const &other) const { return super::find(other) != npos; }

	inline bool BeginsWith(ByteString const &other) const { return !super::compare(0, other.size(), other); }
	inline bool EndsWith(ByteString const &other) const { return !super::compare(size() - other.size(), other.size(), other); }

	using Split = SplitBase<ByteString>;
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, pos, super::find(ch, pos), 1, false); }
	inline Split SplitBy(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, pos, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::rfind(str, pos), str.size(), true); }
	inline Split SplitFromEndByAny(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_of(str, pos), 1, true); }
	inline Split SplitFromEndByNot(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_not_of(str, pos), 1, true); }
private:
	Split SplitSigned(long long int &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
	Split SplitUnsigned(unsigned long long int &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
	Split SplitFloat(double &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
public:
	template<typename T> inline Split SplitSigned(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		long long int value = 0;
		Split split = SplitSigned(value, pos, set, reset);
		ref = value;
		return split;
	}
	template<typename T> inline Split SplitUnsigned(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		unsigned long long int value = 0;
		Split split = SplitUnsigned(value, pos, set, reset);
		ref = value;
		return split;
	}
	template<typename T> inline Split SplitFloat(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		double value = 0;
		Split split = SplitFloat(value, pos, set, reset);
		ref = value;
		return split;
	}

	inline Split SplitNumber(short int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(long int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(long long int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned short int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned long int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned long long int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(float &ref, size_t pos = 0) const { return SplitFloat(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(double &ref, size_t pos = 0) const { return SplitFloat(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }

	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(short int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(long long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned short int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned long long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(float &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitFloat(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(double &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitFloat(ref, pos, set, reset); }

	template<typename T> T ToNumber(bool noThrow = false) const
	{
		T value = T();
		Split split = SplitNumber(value);
		if(split.PositionBefore() != size())
			return noThrow ? T() : throw std::runtime_error("Not a number");
		return value;
	}
	template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline T ToNumber(Format::FlagsOverride<void, set, reset> fmt, bool noThrow = false) const
	{
		T value = T();
		Split split = SplitNumber(value, fmt);
		if(split.PositionBefore() != size())
			return noThrow ? T() : throw std::runtime_error("Not a number");
		return value;
	}

	std::vector<ByteString> PartitionBy(value_type ch, bool includeEmpty = false) const;
	std::vector<ByteString> PartitionBy(ByteString const &str, bool includeEmpty = false) const;
	std::vector<ByteString> PartitionByAny(ByteString const &str, bool includeEmpty = false) const;

	ByteString &Substitute(ByteString const &needle, ByteString const &replacement);

	inline ByteString &Insert(size_t pos, ByteString const &str) { super::insert(pos, str); return *this; }
	inline ByteString &Erase(size_t pos, size_t count) { super::erase(pos, count); return *this; }
	inline ByteString &EraseBetween(size_t from, size_t to) { if(from < to) super::erase(from, to - from); return *this; }

	inline ByteString ToLower() const
	{
		std::locale const &loc = std::locale::classic();
		ByteString value(*this);
		for(value_type &ch : value)
			ch = std::tolower(ch, loc);
		return value;
	}

	inline ByteString ToUpper() const
	{
		std::locale const &loc = std::locale::classic();
		ByteString value(*this);
		for(value_type &ch : value)
			ch = std::toupper(ch, loc);
		return value;
	}

	String FromUtf8(bool ignoreError = true) const;
	inline String FromAscii() const;
	template<typename... Ts> static ByteString Build(Ts&&... args);
};

inline ByteString operator+(ByteString const &lhs, ByteString const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(ByteString const &lhs, ByteString &&rhs) { return static_cast<std::basic_string<char> const &>(lhs) + static_cast<std::basic_string<char> &&>(rhs); }
inline ByteString operator+(ByteString &&lhs, ByteString const &rhs) { return static_cast<std::basic_string<char> &&>(lhs) + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(ByteString &&lhs, ByteString &&rhs) { return static_cast<std::basic_string<char> &&>(lhs) + static_cast<std::basic_string<char> &&>(rhs); }
inline ByteString operator+(ByteString const &lhs, std::basic_string<char> const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) + rhs; }
inline ByteString operator+(ByteString const &lhs, std::basic_string<char> &&rhs) { return static_cast<std::basic_string<char> const &>(lhs) + std::move(rhs); }
inline ByteString operator+(ByteString &&lhs, std::basic_string<char> const &rhs) { return static_cast<std::basic_string<char> &&>(lhs) + rhs; }
inline ByteString operator+(ByteString &&lhs, std::basic_string<char> &&rhs) { return static_cast<std::basic_string<char> &&>(lhs) + std::move(rhs); }
inline ByteString operator+(ByteString const &lhs, ByteString::value_type rhs) { return static_cast<std::basic_string<char> const &>(lhs) + rhs; }
inline ByteString operator+(ByteString &&lhs, ByteString::value_type rhs) { return static_cast<std::basic_string<char> &&>(lhs) + rhs; }
inline ByteString operator+(ByteString const &lhs, ByteString::value_type const *rhs) { return static_cast<std::basic_string<char> const &>(lhs) + rhs; }
inline ByteString operator+(ByteString &&lhs, ByteString::value_type const *rhs) { return static_cast<std::basic_string<char> &&>(lhs) + rhs; }
inline ByteString operator+(std::basic_string<char> const &lhs, ByteString const &rhs) { return lhs + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(std::basic_string<char> const &lhs, ByteString &&rhs) { return lhs + static_cast<std::basic_string<char> &&>(rhs); }
inline ByteString operator+(std::basic_string<char> &&lhs, ByteString const &rhs) { return std::move(lhs) + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(std::basic_string<char> &&lhs, ByteString &&rhs) { return std::move(lhs) + static_cast<std::basic_string<char> &&>(rhs); }
inline ByteString operator+(ByteString::value_type lhs, ByteString const &rhs) { return lhs + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(ByteString::value_type lhs, ByteString &&rhs) { return lhs + static_cast<std::basic_string<char> &&>(rhs); }
inline ByteString operator+(ByteString::value_type const *lhs, ByteString const &rhs) { return lhs + static_cast<std::basic_string<char> const &>(rhs); }
inline ByteString operator+(ByteString::value_type const *lhs, ByteString &&rhs) { return lhs + static_cast<std::basic_string<char> &&>(rhs); }

inline bool operator==(ByteString const &lhs, ByteString const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) == static_cast<std::basic_string<char> const &>(rhs); }
inline bool operator==(ByteString const &lhs, std::basic_string<char> const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) == rhs; }
inline bool operator==(ByteString const &lhs, ByteString::value_type const *rhs) { return static_cast<std::basic_string<char> const &>(lhs) == rhs; }
inline bool operator==(std::basic_string<char> const &lhs, ByteString const &rhs) { return lhs == static_cast<std::basic_string<char> const &>(rhs); }
inline bool operator==(ByteString::value_type const *lhs, ByteString const &rhs) { return lhs == static_cast<std::basic_string<char> const &>(rhs); }

inline bool operator!=(ByteString const &lhs, ByteString const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) != static_cast<std::basic_string<char> const &>(rhs); }
inline bool operator!=(ByteString const &lhs, std::basic_string<char> const &rhs) { return static_cast<std::basic_string<char> const &>(lhs) != rhs; }
inline bool operator!=(ByteString const &lhs, ByteString::value_type const *rhs) { return static_cast<std::basic_string<char> const &>(lhs) != rhs; }
inline bool operator!=(std::basic_string<char> const &lhs, ByteString const &rhs) { return lhs != static_cast<std::basic_string<char> const &>(rhs); }
inline bool operator!=(ByteString::value_type const *lhs, ByteString const &rhs) { return lhs != static_cast<std::basic_string<char> const &>(rhs); }

class String : public std::basic_string<char32_t>
{
	using super = std::basic_string<char32_t>;
public:
	inline String(): super() {}
	inline String(value_type ch): super(1, ch) {}
	inline String(size_type count, value_type ch): super(count, ch) {}
	inline String(value_type const *ch, size_type count): super(ch, count) {}
	inline String(value_type const *ch): super(ch) {}
	template<class It> inline String(It first, It last): super(first, last) {}
	inline String(super const &other): super(other) {}
	inline String(super &&other): super(std::move(other)) {}
	inline String(String const &other): super(other) {}
	inline String(String &&other): super(std::move(other)) {}
	template<size_t N> inline String(ByteString::value_type const (&ch)[N]): super(ByteString(ch, N - 1).FromAscii()) {}

	inline String &operator=(String const &other) { super::operator=(other); return *this; }
	inline String &operator=(String &&other) { super::operator=(other); return *this; }

	inline String &operator+=(String const &other) { super::operator+=(other); return *this; }
	inline String &operator+=(String &&other) { super::operator+=(std::move(other)); return *this; }

	inline String Substr(size_t pos = 0, size_t count = npos) const { return super::substr(pos, count); }
	inline String SubstrFromEnd(size_t rpos = 0, size_t rcount = npos) const { return super::substr(rcount == npos || rcount > rpos ? 0 : rpos - rcount, size() - rpos); }
	inline String Between(size_t from, size_t to) const { return to == npos ? super::substr(from) : from >= to ? String() : super::substr(from, to - from); }

	inline bool Contains(value_type ch) const { return super::find(ch) != npos; }
	inline bool Contains(String const &other) const { return super::find(other) != npos; }

	inline bool BeginsWith(String const &other) const { return !super::compare(0, other.size(), other); }
	inline bool EndsWith(String const &other) const { return !super::compare(size() - other.size(), other.size(), other); }

	using Split = SplitBase<String>;
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, pos, super::find(ch, pos), 1, false); }
	inline Split SplitBy(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, pos, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(String const &str, size_t pos = npos) const { return Split(*this, pos, super::rfind(str, pos), str.size(), true); }
	inline Split SplitFromEndByAny(String const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_of(str, pos), 1, true); }
	inline Split SplitFromEndByNot(String const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_not_of(str, pos), 1, true); }
private:
	Split SplitSigned(long long int &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
	Split SplitUnsigned(unsigned long long int &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
	Split SplitFloat(double &, size_t, std::ios_base::fmtflags, std::ios_base::fmtflags) const;
public:
	template<typename T> inline Split SplitSigned(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		long long int value = 0;
		Split split = SplitSigned(value, pos, set, reset);
		ref = value;
		return split;
	}
	template<typename T> inline Split SplitUnsigned(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		unsigned long long int value = 0;
		Split split = SplitUnsigned(value, pos, set, reset);
		ref = value;
		return split;
	}
	template<typename T> inline Split SplitFloat(T &ref, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
	{
		double value = 0;
		Split split = SplitFloat(value, pos, set, reset);
		ref = value;
		return split;
	}

	inline Split SplitNumber(short int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(long int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(long long int &ref, size_t pos = 0) const { return SplitSigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned short int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned long int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(unsigned long long int &ref, size_t pos = 0) const { return SplitUnsigned(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(float &ref, size_t pos = 0) const { return SplitFloat(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }
	inline Split SplitNumber(double &ref, size_t pos = 0) const { return SplitFloat(ref, pos, std::ios_base::fmtflags(), std::ios_base::fmtflags()); }

	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(short int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(long long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitSigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned short int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(unsigned long long int &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitUnsigned(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(float &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitFloat(ref, pos, set, reset); }
	template<std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline Split SplitNumber(double &ref, Format::FlagsOverride<void, set, reset>, size_t pos = 0) const { return SplitFloat(ref, pos, set, reset); }

	template<typename T> T ToNumber(bool noThrow = false) const
	{
		T value = T();
		Split split = SplitNumber(value);
		if(split.PositionBefore() != size())
			return noThrow ? T() : throw std::runtime_error("Not a number");
		return value;
	}
	template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline T ToNumber(Format::FlagsOverride<void, set, reset> fmt, bool noThrow = false) const
	{
		T value = T();
		Split split = SplitNumber(value, fmt);
		if(split.PositionBefore() != size())
			return noThrow ? T() : throw std::runtime_error("Not a number");
		return value;
	}

	std::vector<String> PartitionBy(value_type ch, bool includeEmpty = false) const;
	std::vector<String> PartitionBy(String const &str, bool includeEmpty = false) const;
	std::vector<String> PartitionByAny(String const &str, bool includeEmpty = false) const;

	String &Substitute(String const &needle, String const &replacement);

	inline String &Insert(size_t pos, String const &str) { super::insert(pos, str); return *this; }
	inline String &Erase(size_t pos, size_t count) { super::erase(pos, count); return *this; }
	inline String &EraseBetween(size_t from, size_t to) { if(from < to) super::erase(from, to - from); return *this; }

	inline String ToLower() const
	{
		std::locale const &loc = std::locale::classic();
		String value(*this);
		for(value_type &ch : value)
			if(ch <= std::numeric_limits<ByteString::value_type>::max())
				ch = std::tolower(static_cast<ByteString::value_type>(ch), loc);
		return value;
	}

	inline String ToUpper() const
	{
		std::locale const &loc = std::locale::classic();
		String value(*this);
		for(value_type &ch : value)
			if(ch <= std::numeric_limits<ByteString::value_type>::max())
				ch = std::toupper(static_cast<ByteString::value_type>(ch), loc);
		return value;
	}

	ByteString ToUtf8() const;
	ByteString ToAscii() const;
	template<typename... Ts> static String Build(Ts&&... args);
};

inline String operator+(String const &lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String const &lhs, String &&rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String &&lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String &&lhs, String &&rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String const &lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + rhs; }
inline String operator+(String const &lhs, std::basic_string<char32_t> &&rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + std::move(rhs); }
inline String operator+(String &&lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + rhs; }
inline String operator+(String &&lhs, std::basic_string<char32_t> &&rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + std::move(rhs); }
inline String operator+(String const &lhs, String::value_type rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + rhs; }
inline String operator+(String &&lhs, String::value_type rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + rhs; }
inline String operator+(String const &lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) + rhs; }
inline String operator+(String &&lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> &&>(lhs) + rhs; }
inline String operator+(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(std::basic_string<char32_t> const &lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(std::basic_string<char32_t> &&lhs, String const &rhs) { return std::move(lhs) + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(std::basic_string<char32_t> &&lhs, String &&rhs) { return std::move(lhs) + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String::value_type lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String::value_type lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String::value_type const *lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String::value_type const *lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
template<size_t N> inline String operator+(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) + ByteString(rhs).FromAscii(); }
template<size_t N> inline String operator+(String &&lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> &&>(lhs) + ByteString(rhs).FromAscii(); }
template<size_t N> inline String operator+(ByteString::value_type const (&lhs)[N], String const &rhs) { return ByteString(lhs).FromAscii() + static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline String operator+(ByteString::value_type const (&lhs)[N], String &&rhs) { return ByteString(lhs).FromAscii() + static_cast<std::basic_string<char32_t> &&>(rhs); }

inline bool operator==(String const &lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator==(String const &lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == rhs; }
inline bool operator==(String const &lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == rhs; }
inline bool operator==(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs == static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator==(String::value_type const *lhs, String const &rhs) { return lhs == static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline bool operator==(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) == ByteString(rhs).FromAscii(); }
template<size_t N> inline bool operator==(ByteString::value_type const (&lhs)[N], String const &rhs) { return ByteString(lhs).FromAscii() == static_cast<std::basic_string<char32_t> const &>(rhs); }

inline bool operator!=(String const &lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator!=(String const &lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != rhs; }
inline bool operator!=(String const &lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != rhs; }
inline bool operator!=(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs != static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator!=(String::value_type const *lhs, String const &rhs) { return lhs != static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline bool operator!=(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) != ByteString(rhs).FromAscii(); }
template<size_t N> inline bool operator!=(ByteString::value_type const (&lhs)[N], String const &rhs) { return ByteString(lhs).FromAscii() != static_cast<std::basic_string<char32_t> const &>(rhs); }

inline String ByteString::FromAscii() const
{
	String destination = String(size(), String::value_type());
	for(size_t i = 0; i < size(); i++)
		destination[i] = String::value_type(std::make_unsigned<ByteString::value_type>::type(operator[](i)));
	return destination;
}

inline ByteString String::ToAscii() const
{
	ByteString destination = ByteString(size(), ByteString::value_type());
	for(size_t i = 0; i < size(); i++)
		destination[i] = ByteString::value_type(operator[](i));
	return destination;
}

class ConversionError : public std::runtime_error
{
	static ByteString formatError(ByteString::value_type const *at, ByteString::value_type const *upto);
public:
	inline ConversionError(ByteString::value_type const *at, ByteString::value_type const *upto): std::runtime_error(formatError(at, upto)) {}
	inline ConversionError(bool to): std::runtime_error(to ? "Could not convert to UTF-8" : "Could not convert from UTF-8") {}
};

class ByteStringBuilder
{
	std::vector<ByteString::value_type> buffer;
public:
	std::ios_base::fmtflags flags;
	ByteString::value_type fill;
	size_t width, precision;
	inline ByteStringBuilder(): flags(std::ios_base::skipws | std::ios_base::dec), fill(' '), width(0), precision(6) {}

	void AddChars(ByteString::value_type const *, size_t);
	size_t Size() const { return buffer.size(); }
	ByteString Build() const;

	template<typename T> ByteStringBuilder &operator<<(T) &&= delete;

	template<typename T, typename... Ts> inline ByteStringBuilder &Add(T &&arg, Ts&&... args)
	{
		return (*this << std::forward<T>(arg)).Add(std::forward<Ts>(args)...);
	}
	inline ByteStringBuilder &Add() { return *this; }
};

ByteStringBuilder &operator<<(ByteStringBuilder &, short int);
ByteStringBuilder &operator<<(ByteStringBuilder &, int);
ByteStringBuilder &operator<<(ByteStringBuilder &, long int);
ByteStringBuilder &operator<<(ByteStringBuilder &, long long int);
ByteStringBuilder &operator<<(ByteStringBuilder &, unsigned short int);
ByteStringBuilder &operator<<(ByteStringBuilder &, unsigned int);
ByteStringBuilder &operator<<(ByteStringBuilder &, unsigned long int);
ByteStringBuilder &operator<<(ByteStringBuilder &, unsigned long long int);
ByteStringBuilder &operator<<(ByteStringBuilder &, ByteString::value_type);
ByteStringBuilder &operator<<(ByteStringBuilder &, ByteString::value_type const *);
ByteStringBuilder &operator<<(ByteStringBuilder &, ByteString const &);
ByteStringBuilder &operator<<(ByteStringBuilder &, float);
ByteStringBuilder &operator<<(ByteStringBuilder &, double);

template<typename... Ts> ByteString ByteString::Build(Ts&&... args)
{
	ByteStringBuilder b;
	b.Add(std::forward<Ts>(args)...);
	return b.Build();
}

class StringBuilder
{
	std::vector<String::value_type> buffer;
public:
	std::ios_base::fmtflags flags;
	String::value_type fill;
	size_t width, precision;
	inline StringBuilder(): flags(std::ios_base::skipws | std::ios_base::dec), fill(' '), width(0), precision(6) {}

	void AddChars(String::value_type const *, size_t);
	size_t Size() const { return buffer.size(); }
	String Build() const;

	template<typename T> StringBuilder &operator<<(T) = delete;

	template<typename T, typename... Ts> inline StringBuilder &Add(T &&arg, Ts&&... args)
	{
		return (*this << std::forward<T>(arg)).Add(std::forward<Ts>(args)...);
	}
	inline StringBuilder &Add() { return *this; }
};

StringBuilder &operator<<(StringBuilder &, short int);
StringBuilder &operator<<(StringBuilder &, int);
StringBuilder &operator<<(StringBuilder &, long int);
StringBuilder &operator<<(StringBuilder &, long long int);
StringBuilder &operator<<(StringBuilder &, unsigned short int);
StringBuilder &operator<<(StringBuilder &, unsigned int);
StringBuilder &operator<<(StringBuilder &, unsigned long int);
StringBuilder &operator<<(StringBuilder &, unsigned long long int);
StringBuilder &operator<<(StringBuilder &, ByteString::value_type);
StringBuilder &operator<<(StringBuilder &, String::value_type);
StringBuilder &operator<<(StringBuilder &, String::value_type const *);
StringBuilder &operator<<(StringBuilder &, String const &);
StringBuilder &operator<<(StringBuilder &, float);
StringBuilder &operator<<(StringBuilder &, double);
template<size_t N> StringBuilder &operator<<(StringBuilder &b, ByteString::value_type const (&data)[N]) { return b << ByteString(data).FromUtf8(); }

template<typename... Ts> String String::Build(Ts&&... args)
{
	StringBuilder b;
	b.Add(std::forward<Ts>(args)...);
	return b.Build();
}

#include "common/Format.h"
