#ifndef TPT_STRING
#define TPT_STRING

#include <stdexcept>
#include <sstream>
#include <string>

class String;

template<typename T> class SplitBase
{
	T const &parent;
	size_t posBefore;
	size_t posAfter;
	bool reverse;

	inline SplitBase(T const &_parent, size_t _posBefore, size_t offset, bool _reverse):
		parent(_parent),
		posBefore(_posBefore),
		posAfter(_posBefore == T::npos ? T::npos : _posBefore + offset),
		reverse(_reverse)
	{}
public:
	inline T Before(bool includeSeparator = false) const
	{
		if(posBefore == T::npos)
			return reverse ? T() : parent;
		return parent.Substr(0, includeSeparator ? posAfter : posBefore);
	}
	inline T After(bool includeSeparator = false) const
	{
		if(posBefore == T::npos)
			return reverse ? parent : T();
		return parent.Substr(includeSeparator ? posBefore : posAfter);
	}

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
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, super::find(ch, pos), 1, false); }
	inline Split SplitBy(ByteString const &str, size_t pos = 0) const { return Split(*this, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(ByteString const &str, size_t pos = 0) const { return Split(*this, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(ByteString const &str, size_t pos = 0) const { return Split(*this, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(ByteString const &str, size_t pos = npos) const { return Split(*this, super::find(str, pos), str.size(), true); }
	inline Split SplitFromEndByAny(ByteString const &str, size_t pos = npos) const { return Split(*this, super::find_last_of(str, pos), 1, true); }
	inline Split SplitFromEndByNot(ByteString const &str, size_t pos = npos) const { return Split(*this, super::find_last_not_of(str, pos), 1, true); }

	inline ByteString &Substitute(ByteString const &needle, ByteString const &replacement)
	{
		size_t needleSize = needle.size();
		size_t replacementSize = replacement.size();
		size_t at = super::find(needle);
		while(at != npos)
		{
			super::replace(at, needleSize, replacement);
			at += replacementSize + !needleSize;
			at = super::find(needle, at);
		}
		return *this;
	}

public:

	class ConversionError : public std::runtime_error
	{
		static std::string formatError(value_type const *at, value_type const *upto);
	public:
		inline ConversionError(value_type const *at, value_type const *upto): std::runtime_error(formatError(at, upto)) {}
		inline ConversionError(bool to): std::runtime_error(to ? "Could not convert to UTF-8" : "Could not convert from UTF-8") {}
	};

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
	inline Split SplitBy(value_type ch, size_t pos = 0) const { return Split(*this, super::find(ch, pos), 1, false); }
	inline Split SplitBy(String const &str, size_t pos = 0) const { return Split(*this, super::find(str, pos), str.size(), false); }
	inline Split SplitByAny(String const &str, size_t pos = 0) const { return Split(*this, super::find_first_of(str, pos), 1, false); }
	inline Split SplitByNot(String const &str, size_t pos = 0) const { return Split(*this, super::find_first_not_of(str, pos), 1, false); }
	inline Split SplitFromEndBy(value_type ch, size_t pos = npos) const { return Split(*this, super::rfind(ch, pos), 1, true); }
	inline Split SplitFromEndBy(String const &str, size_t pos = npos) const { return Split(*this, super::find(str, pos), str.size(), true); }
	inline Split SplitFromEndByAny(String const &str, size_t pos = npos) const { return Split(*this, super::find_last_of(str, pos), 1, true); }
	inline Split SplitFromEndByNot(String const &str, size_t pos = npos) const { return Split(*this, super::find_last_not_of(str, pos), 1, true); }

	inline String &Substitute(String const &needle, String const &replacement)
	{
		size_t needleSize = needle.size();
		size_t replacementSize = replacement.size();
		size_t at = super::find(needle);
		while(at != npos)
		{
			super::replace(at, needleSize, replacement);
			at += replacementSize + !needleSize;
			at = super::find(needle, at);
		}
		return *this;
	}
	
	inline String &Insert(size_t pos, String const &str) { super::insert(pos, str); return *this; }
	inline String &Erase(size_t pos, size_t count) { super::erase(pos, count); return *this; }
	inline String &EraseBetween(size_t from, size_t to) { if(from < to) super::erase(from, to - from); return *this; }

	inline bool operator==(String const &other) { return std::basic_string<char32_t>(*this) == other; }
	inline bool operator!=(String const &other) { return std::basic_string<char32_t>(*this) != other; }

	ByteString ToUtf8() const;
	ByteString ToAscii() const;

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
#endif
