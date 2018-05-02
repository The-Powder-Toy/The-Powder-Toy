#pragma once

#include <stdexcept>
#include <sstream>
#include <vector>
#include <string>
#include <ios>

class ByteStringBuilder;
class String;
class StringBuilder;
namespace Format { template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> struct FlagsOverride; }

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
	inline ByteString Between(size_t from, size_t to) const { return from >= to ? ByteString() : super::substr(from, to - from); }

	inline bool Contains(value_type ch) const { return super::find(ch) != npos; }
	inline bool Contains(ByteString const &other) { return super::find(other) != npos; }

	inline bool BeginsWith(ByteString const &other) const { return super::compare(0, other.size(), other); }
	inline bool EndsWith(ByteString const &other) const { return super::compare(size() - other.size(), other.size(), other); }

	using Split = SplitBase<ByteString>;
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, pos, super::find(ch, pos), 1, false); }
	inline Split SplitBy(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(ByteString const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, pos, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::find(str, pos), str.size(), true); }
	inline Split SplitFromEndByAny(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_of(str, pos), 1, true); }
	inline Split SplitFromEndByNot(ByteString const &str, size_t pos = npos) const { return Split(*this, pos, super::find_last_not_of(str, pos), 1, true); }

	std::vector<ByteString> PartitionBy(value_type ch, bool includeEmpty = false) const;
	std::vector<ByteString> PartitionBy(ByteString const &str, bool includeEmpty = false) const;
	std::vector<ByteString> PartitionByAny(ByteString const &str, bool includeEmpty = false) const;

	ByteString &Substitute(ByteString const &needle, ByteString const &replacement);

	inline ByteString &Insert(size_t pos, ByteString const &str) { super::insert(pos, str); return *this; }
	inline ByteString &Erase(size_t pos, size_t count) { super::erase(pos, count); return *this; }
	inline ByteString &EraseBetween(size_t from, size_t to) { if(from < to) super::erase(from, to - from); return *this; }

	String FromUtf8(bool ignoreError = true) const;
	inline String FromAscii() const;

	using Stream = std::basic_stringstream<value_type>;
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
	inline String Between(size_t from, size_t to) const { return from >= to ? String() : super::substr(from, to - from); }

	inline bool Contains(value_type ch) const { return super::find(ch) != npos; }
	inline bool Contains(String const &other) const { return super::find(other) != npos; }

	inline bool BeginsWith(String const &other) const { return super::compare(0, other.size(), other); }
	inline bool EndsWith(String const &other) const { return super::compare(size() - other.size(), other.size(), other); }

	using Split = SplitBase<String>;
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, pos, super::find(ch, pos), 1, false); }
	inline Split SplitBy(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(String const &str, size_t pos = 0) const { return Split(*this, pos, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, pos, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(String const &str, size_t pos = npos) const { return Split(*this, pos,super::find(str, pos), str.size(), true); }
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

	template<typename T> T ToNumber() const
	{
		T value = T();
		Split split = SplitNumber(value);
		if(split.PositionBefore() != size())
			return T();
		return value;
	}
	template<typename T, std::ios_base::fmtflags set, std::ios_base::fmtflags reset> inline T ToNumber(Format::FlagsOverride<void, set, reset> fmt) const
	{
		T value = T();
		Split split = SplitNumber(value, fmt);
		if(split.PositionBefore() != size())
			return T();
		return value;
	}

	std::vector<String> PartitionBy(value_type ch, bool includeEmpty = false) const;
	std::vector<String> PartitionBy(String const &str, bool includeEmpty = false) const;
	std::vector<String> PartitionByAny(String const &str, bool includeEmpty = false) const;

	String &Substitute(String const &needle, String const &replacement);
	
	inline String &Insert(size_t pos, String const &str) { super::insert(pos, str); return *this; }
	inline String &Erase(size_t pos, size_t count) { super::erase(pos, count); return *this; }
	inline String &EraseBetween(size_t from, size_t to) { if(from < to) super::erase(from, to - from); return *this; }

	ByteString ToUtf8() const;
	ByteString ToAscii() const;
	template<typename... Ts> static String Build(Ts&&... args);

	using Stream = std::basic_stringstream<value_type>;
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
template<size_t N> inline String operator+(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) + std::move(ByteString(rhs).FromAscii()); }
template<size_t N> inline String operator+(String &&lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> &&>(lhs) + std::move(ByteString(rhs).FromAscii()); }
inline String operator+(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(std::basic_string<char32_t> const &lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(std::basic_string<char32_t> &&lhs, String const &rhs) { return std::move(lhs) + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(std::basic_string<char32_t> &&lhs, String &&rhs) { return std::move(lhs) + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String::value_type lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String::value_type lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
inline String operator+(String::value_type const *lhs, String const &rhs) { return lhs + static_cast<std::basic_string<char32_t> const &>(rhs); }
inline String operator+(String::value_type const *lhs, String &&rhs) { return lhs + static_cast<std::basic_string<char32_t> &&>(rhs); }
template<size_t N> inline String operator+(ByteString::value_type const (&lhs)[N], String const &rhs) { return std::move(ByteString(lhs).FromAscii()) + static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline String operator+(ByteString::value_type const (&lhs)[N], String &&rhs) { return std::move(ByteString(lhs).FromAscii()) + static_cast<std::basic_string<char32_t> &&>(rhs); }

inline bool operator==(String const &lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator==(String const &lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == rhs; }
inline bool operator==(String const &lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) == rhs; }
template<size_t N> inline bool operator==(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) == std::move(ByteString(rhs).FromAscii()); }
inline bool operator==(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs == static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator==(String::value_type const *lhs, String const &rhs) { return lhs == static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline bool operator==(ByteString::value_type const (&lhs)[N], String const &rhs) { return std::move(ByteString(lhs).FromAscii()) == static_cast<std::basic_string<char32_t> const &>(rhs); }

inline bool operator!=(String const &lhs, String const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator!=(String const &lhs, std::basic_string<char32_t> const &rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != rhs; }
inline bool operator!=(String const &lhs, String::value_type const *rhs) { return static_cast<std::basic_string<char32_t> const &>(lhs) != rhs; }
template<size_t N> inline bool operator!=(String const &lhs, ByteString::value_type const (&rhs)[N]) { return static_cast<std::basic_string<char32_t> const &>(lhs) != std::move(ByteString(rhs).FromAscii()); }
inline bool operator!=(std::basic_string<char32_t> const &lhs, String const &rhs) { return lhs != static_cast<std::basic_string<char32_t> const &>(rhs); }
inline bool operator!=(String::value_type const *lhs, String const &rhs) { return lhs != static_cast<std::basic_string<char32_t> const &>(rhs); }
template<size_t N> inline bool operator!=(ByteString::value_type const (&lhs)[N], String const &rhs) { return std::move(ByteString(lhs).FromAscii()) != static_cast<std::basic_string<char32_t> const &>(rhs); }

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

class StringBuilder
{
	std::vector<String::value_type> buffer; // TODO: std::list<std::vector<String::value_type> > ?
public:
	std::ios_base::fmtflags flags;
	String::value_type fill;
	size_t width, precision;
	inline StringBuilder(): flags(std::ios_base::skipws | std::ios_base::dec), fill(' '), width(0), precision(6) {}

	void AddChars(String::value_type const *, size_t);
	size_t Size() const { return buffer.size(); }
	String Build() const;

	template<typename T> StringBuilder &operator<<(T) = delete;

	template<typename T, typename... Ts> StringBuilder &Add(T &&arg, Ts&&... args)
	{
		return (*this << std::forward<T>(arg)).Add(std::forward<Ts>(args)...);
	}
	StringBuilder &Add() { return *this; }
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
