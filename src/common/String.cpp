#include <sstream>
#include <vector>
#include <locale>
#include <limits>
#include <stdexcept>

#include "common/tpt-thread.h"
#include "String.h"

ByteString ConversionError::formatError(ByteString::value_type const *at, ByteString::value_type const *upto)
{
	std::stringstream ss;
	ss << "Could not convert sequence to UTF-8:";
	for(int i = 0; i < 4 && at + i < upto; i++)
		ss << " " << std::hex << (unsigned int)std::make_unsigned<ByteString::value_type>::type(at[i]);
	return ss.str();
}

std::vector<ByteString> ByteString::PartitionBy(value_type ch, bool includeEmpty) const
{
	std::vector<ByteString> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitBy(ch, at);
		ByteString part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

std::vector<ByteString> ByteString::PartitionBy(ByteString const &str, bool includeEmpty) const
{
	std::vector<ByteString> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitBy(str, at);
		ByteString part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

std::vector<ByteString> ByteString::PartitionByAny(ByteString const &str, bool includeEmpty) const
{
	std::vector<ByteString> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitByAny(str, at);
		ByteString part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

ByteString &ByteString::Substitute(ByteString const &needle, ByteString const &replacement)
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

// The STL codecvt interfaces aren't very portable and behave wildly
// differntly on edge cases.
// Refer to Table 3.1 of the Unicode Standard version 3.1.0+

String ByteString::FromUtf8(bool ignoreError) const
{
	std::vector<String::value_type> destination;
	destination.reserve(size());
	std::make_unsigned<value_type>::type const *from = reinterpret_cast<std::make_unsigned<value_type>::type const *>(data());
	for(size_t i = 0; i < size(); )
	{
		if(from[i] < 0x80)
		{
			destination.push_back(from[i]);
			i += 1;
			continue;
		}
		else if(from[i] >= 0xC2 && from[i] < 0xE0)
		{
			if(i + 1 < size() && from[i + 1] >= 0x80 && from[i + 1] < 0xC0)
			{
				destination.push_back((from[i] & 0x1F) << 6 | (from[i + 1] & 0x3F));
				i += 2;
				continue;
			}
		}
		else if(from[i] >= 0xE0 && from[i] < 0xF0)
		{
			if(i + 1 < size() && from[i + 1] >= (from[i] == 0xE0 ? 0xA0 : 0x80) && from[i + 1] < 0xC0)
				if(i + 2 < size() && from[i + 2] >= 0x80 && from[i + 2] < 0xC0)
				{
					destination.push_back((from[i] & 0x0F) << 12 | (from[i + 1] & 0x3F) << 6 | (from[i + 2] & 0x3F));
					i += 3;
					continue;
				}
		}
		else if(from[i] >= 0xF0 && from[i] < 0xF5)
		{
			if(i + 1 < size() && from[i + 1] >= (from[i] == 0xF0 ? 0x90 : 0x80) && from[i + 1] < (from[i] == 0xF4 ? 0x90 : 0xC0))
				if(i + 2 < size() && from[i + 2] >= 0x80 && from[i + 2] < 0xC0)
					if(i + 3 < size() && from[i + 3] >= 0x80 && from[i + 3] < 0xC0)
					{
						destination.push_back((from[i] & 0x07) << 18 | (from[i + 1] & 0x3F) << 12 | (from[i + 2] & 0x3F) | (from[i + 3] & 0x3F));
						i += 4;
						continue;
					}
		}
		if(ignoreError)
			destination.push_back(from[i++]);
		else
			throw ConversionError(data() + i, data() + size());
	}
	return String(destination.data(), destination.size());
}

std::vector<String> String::PartitionBy(value_type ch, bool includeEmpty) const
{
	std::vector<String> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitBy(ch, at);
		String part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

std::vector<String> String::PartitionBy(String const &str, bool includeEmpty) const
{
	std::vector<String> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitBy(str, at);
		String part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

std::vector<String> String::PartitionByAny(String const &str, bool includeEmpty) const
{
	std::vector<String> result;
	size_t at = 0;
	while(true)
	{
		Split split = SplitByAny(str, at);
		String part = split.Before();
		if(includeEmpty || part.size())
			result.push_back(part);
		at = split.PositionAfter();
		if(!split)
			break;
	}
	return result;
}

String &String::Substitute(String const &needle, String const &replacement)
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

ByteString String::ToUtf8() const
{
	std::vector<std::make_unsigned<ByteString::value_type>::type> destination;
	destination.reserve(size() * 2);
	value_type const *from = data();
	for(size_t i = 0; i < size(); i++)
	{
		if(from[i] >= 0 && from[i] < 0x80)
		{
			destination.push_back(from[i]);
		}
		else if(from[i] < 0x800)
		{
			destination.push_back(0xC0 | (from[i] >> 6));
			destination.push_back(0x80 | (from[i] & 0x3F));
		}
		else if(from[i] < 0x10000)
		{
			destination.push_back(0xE0 | (from[i] >> 12));
			destination.push_back(0x80 | ((from[i] >> 6) & 0x3F));
			destination.push_back(0x80 | (from[i] & 0x3F));
		}
		else if(from[i] <= 0x10FFFF)
		{
			destination.push_back(0xF0 | (from[i] >> 18));
			destination.push_back(0x80 | ((from[i] >> 12) & 0x3F));
			destination.push_back(0x80 | ((from[i] >> 6) & 0x3F));
			destination.push_back(0x80 | (from[i] & 0x3F));
		}
		else
			throw ConversionError(true);
	}
	return ByteString(reinterpret_cast<ByteString::value_type const *>(destination.data()), destination.size());
}

/*
	Due to unknown reasons, the STL basically doesn't support string-number
	conversions for char32_t strings. Under the hood all stream objects use
	the so-called locale facets to do number formatting and parsing. As the
	name implies the facets can depend on the currently chosen locale, but
	they are also specialized by the type of characters that are used in
	the strings that are written/read.

	Of particular interest are the std::num_put<T> and std::num_get<T>
	facets. In accordance with the standard the two class templates are
	defined, and then specialized to char and wchar_t types. But the
	generic class template does not implement all the necessary methods,
	leaving you with undefined reference errors. Manually providing
	implementations for such methods is a not a portable solution.
	Therefore we provide our own number reading/writing interface, detached
	from std::basic_stringstream.

	We would nevertheless like to avoid writing all the conversion code
	ourselves and use STL as much as possible. As it turns out std::num_put
	and std::num_get are too wired into std::ios_base and thus are unusable
	in separation from an STL stream object.

	A hacky but simple solution is to create a static thread-local
	std::wstringstream initialized to the C locale (setting the locale of a
	temporarily created stream every time might be too expensive). Number
	serialization then simply uses operator<< and then manually widens the
	produced wchar_t's into char32_t's. Number parsing is more tricky and
	narrows only a prefix of the parsed string: it selects only characters
	that could be a part of a number as in "Stage 2" in
	[facet.num.get.virtuals], narrows them and uses operator>>. The number
	of characters consumed is used to take an offset into the original
	string.

	A std::stringstream is added in the same way for symmetry with
	ByteStringStream and follows the same protocol except it doesn't
	perform and narrowing or widening.

	The nice thing above the *_wchar functions immediately below is that on
	platforms where wchar_t has 32 bits these should be a no-op.
*/

inline String::value_type widen_wchar(wchar_t ch)
{
	return std::make_unsigned<wchar_t>::type(ch);
}

inline bool representable_wchar(String::value_type ch)
{
	return ch < String::value_type(std::make_unsigned<wchar_t>::type(std::numeric_limits<wchar_t>::max()));
}

inline wchar_t narrow_wchar(String::value_type ch)
{
	return wchar_t(ch);
}

char const numberChars[] = "-.+0123456789ABCDEFXabcdefx";
ByteString numberByteString(numberChars);
String numberString(numberChars);

struct LocaleImpl
{
	std::basic_stringstream<char> stream;
	std::basic_stringstream<wchar_t> wstream;

	LocaleImpl()
	{
		stream.imbue(std::locale::classic());
		wstream.imbue(std::locale::classic());
	}

	inline void PrepareStream(ByteStringBuilder &b)
	{
		stream.flags(b.flags);
		stream.width(b.width);
		stream.precision(b.precision);
		stream.fill(b.fill);
		stream.clear();
	}

	inline void PrepareStream(ByteString const &str, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset)
	{
		stream.flags((std::ios_base::dec & ~ reset) | set);
		std::basic_string<char> bstr;
		while(pos < str.size() && numberByteString.Contains(str[pos]))
			bstr.push_back(narrow_wchar(str[pos++]));
		stream.str(bstr);
		stream.clear();
	}

	inline void FlushStream(ByteStringBuilder &b)
	{
		std::basic_string<char> str = stream.str();
		b.AddChars(str.data(), str.size());
		stream.str(std::basic_string<char>());
	}

	inline void FlushStream()
	{
		stream.str(std::basic_string<char>());
	}

	inline void PrepareWStream(StringBuilder &b)
	{
		wstream.flags(b.flags);
		wstream.width(b.width);
		wstream.precision(b.precision);
		wstream.fill(b.fill);
		wstream.clear();
	}

	inline void PrepareWStream(String const &str, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset)
	{
		wstream.flags((std::ios_base::dec & ~ reset) | set);
		std::basic_string<wchar_t> wstr;
		while(pos < str.size() && representable_wchar(str[pos]) && numberString.Contains(str[pos]))
			wstr.push_back(narrow_wchar(str[pos++]));
		wstream.str(wstr);
		wstream.clear();
	}

	inline void FlushWStream(StringBuilder &b)
	{
		std::basic_string<wchar_t> wstr = wstream.str();
		std::vector<String::value_type> chars; // operator new?
		chars.reserve(wstr.size());
		for(wchar_t ch : wstream.str())
			chars.push_back(widen_wchar(ch));
		b.AddChars(chars.data(), chars.size());
		wstream.str(std::basic_string<wchar_t>());
	}

	inline void FlushWStream()
	{
		wstream.str(std::basic_string<wchar_t>());
	}
};

static void destroyLocaleImpl(void *ptr)
{
	delete static_cast<LocaleImpl *>(ptr);
}

static pthread_once_t localeOnce = PTHREAD_ONCE_INIT;
static pthread_key_t localeKey;

static void createLocaleKey()
{
	if(int error = pthread_key_create(&localeKey, destroyLocaleImpl))
		throw std::system_error(error, std::system_category(), "Could not create TLS key for LocaleImpl");
}

static LocaleImpl *getLocaleImpl()
{
	pthread_once(&localeOnce, createLocaleKey);
	void *ptr = pthread_getspecific(localeKey);
	if(!ptr)
	{
		ptr = static_cast<void *>(new LocaleImpl());
		if(int error = pthread_setspecific(localeKey, ptr))
			throw std::system_error(error, std::system_category(), "Could not put LocaleImpl into TLS");
	}
	return static_cast<LocaleImpl *>(ptr);
}

ByteString ByteStringBuilder::Build() const
{
	return ByteString(buffer.begin(), buffer.end());
}

void ByteStringBuilder::AddChars(ByteString::value_type const *data, size_t count)
{
	buffer.reserve(buffer.size() + count);
	buffer.insert(buffer.end(), data, data + count);
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, short int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, long long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned short int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned long long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, ByteString::value_type data)
{
	b.AddChars(&data, 1);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, ByteString::value_type const *data)
{
	return b << ByteString(data);
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, ByteString const &data)
{
	b.AddChars(data.data(), data.size());
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, float data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, double data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(b);
	impl->stream << data;
	impl->FlushStream(b);
	return b;
}

ByteString::Split ByteString::SplitSigned(long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(*this, pos, set, reset);
	impl->stream >> value;
	if(impl->stream.fail())
	{
		impl->FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->stream.clear();
	Split split(*this, pos, pos + impl->stream.tellg(), 0, false);
	impl->FlushStream();
	return split;
}

ByteString::Split ByteString::SplitUnsigned(unsigned long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(*this, pos, set, reset);
	impl->stream >> value;
	if(impl->stream.fail())
	{
		impl->FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->stream.clear();
	Split split(*this, pos, pos + impl->stream.tellg(), 0, false);
	impl->FlushStream();
	return split;
}

ByteString::Split ByteString::SplitFloat(double &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareStream(*this, pos, set, reset);
	impl->stream >> value;
	if(impl->stream.fail())
	{
		impl->FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->stream.clear();
	Split split(*this, pos, pos + impl->stream.tellg(), 0, false);
	impl->FlushStream();
	return split;
}

String StringBuilder::Build() const
{
	return String(buffer.begin(), buffer.end());
}

void StringBuilder::AddChars(String::value_type const *data, size_t count)
{
	buffer.reserve(buffer.size() + count);
	buffer.insert(buffer.end(), data, data + count);
}

StringBuilder &operator<<(StringBuilder &b, short int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, long long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned short int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned long long int data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, ByteString::value_type data)
{
	String::value_type ch = data;
	b.AddChars(&ch, 1);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, String::value_type data)
{
	b.AddChars(&data, 1);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, String::value_type const *data)
{
	return b << String(data);
}

StringBuilder &operator<<(StringBuilder &b, String const &data)
{
	b.AddChars(data.data(), data.size());
	return b;
}

StringBuilder &operator<<(StringBuilder &b, float data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, double data)
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(b);
	impl->wstream << data;
	impl->FlushWStream(b);
	return b;
}

String::Split String::SplitSigned(long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(*this, pos, set, reset);
	impl->wstream >> value;
	if(impl->wstream.fail())
	{
		impl->FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->wstream.clear();
	Split split(*this, pos, pos + impl->wstream.tellg(), 0, false);
	impl->FlushWStream();
	return split;
}

String::Split String::SplitUnsigned(unsigned long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(*this, pos, set, reset);
	impl->wstream >> value;
	if(impl->wstream.fail())
	{
		impl->FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->wstream.clear();
	Split split(*this, pos, pos + impl->wstream.tellg(), 0, false);
	impl->FlushWStream();
	return split;
}

String::Split String::SplitFloat(double &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl *impl = getLocaleImpl();
	impl->PrepareWStream(*this, pos, set, reset);
	impl->wstream >> value;
	if(impl->wstream.fail())
	{
		impl->FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	impl->wstream.clear();
	Split split(*this, pos, pos + impl->wstream.tellg(), 0, false);
	impl->FlushWStream();
	return split;
}
