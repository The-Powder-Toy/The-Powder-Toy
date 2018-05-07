#include <sstream>
#include <vector>
#include <locale>
#include <codecvt>
#include <limits>

#include "String.h"

ByteString ConversionError::formatError(ByteString::value_type const *at, ByteString::value_type const *upto)
{
	std::stringstream ss;
	ss << "Could not convert sequence to UTF-8:";
	for(int i = 0; i < 4 && at + i < upto; i++)
		ss << " " << std::hex << (unsigned int)std::make_unsigned<ByteString::value_type>::type(at[i]);
	return ss.str();
}

// The STL-packaged standardized UTF-8 conversion interface
static std::codecvt_utf8<String::value_type> convert(1);

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

String ByteString::FromUtf8(bool ignoreError) const
{
	std::vector<String::value_type> destination = std::vector<String::value_type>(size(), String::value_type());
	std::codecvt_utf8<String::value_type>::state_type state;

	ByteString::value_type const *from = data(), *from_next;
	String::value_type *to = destination.data(), *to_next;

	while(true)
	{
		std::codecvt_utf8<String::value_type>::result result = convert.in(state, from, data() + size(), from_next, to, destination.data() + destination.size(), to_next);
		from = from_next;
		to = to_next;
		if(result == std::codecvt_base::ok || result == std::codecvt_base::noconv)
		{
			destination.resize(to - destination.data());
			return String(destination.data(), destination.size());
		}
		else if(result == std::codecvt_base::partial && to == destination.data() + destination.size())
		{
			String::value_type *old_data = destination.data();
			destination.resize(2 * destination.size());
			to = destination.data() + (to - old_data);
		}
		else
		{
			if(!ignoreError)
				throw ConversionError(from, data() + size());

			if(to == destination.data() + destination.size())
			{
				String::value_type *old_data = destination.data();
				destination.resize(2 * destination.size());
				to = destination.data() + (to - old_data);
			}
			*(to++) = std::make_unsigned<ByteString::value_type>::type(*(from++));
		}
	}
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
	std::vector<ByteString::value_type> destination = std::vector<ByteString::value_type>(size(), ByteString::value_type());
	std::codecvt_utf8<String::value_type>::state_type state;

	String::value_type const *from = data(), *from_next;
	ByteString::value_type *to = destination.data(), *to_next;

	while(true)
	{
		std::codecvt_utf8<String::value_type>::result result = convert.out(state, from, data() + size(), from_next, to, destination.data() + destination.size(), to_next);
		from = from_next;
		to = to_next;
		if(result == std::codecvt_base::ok || result == std::codecvt_base::noconv)
		{
			destination.resize(to - destination.data());
			return ByteString(destination.data(), destination.size());
		}
		else if(result == std::codecvt_base::error)
		{
			throw ConversionError(true);
		}
		else if(result == std::codecvt_base::partial)
		{
			ByteString::value_type *old_data = destination.data();
			destination.resize(2 * destination.size());
			to = destination.data() + (to - old_data);
		}
	}
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

static thread_local struct LocaleImpl
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
}
LocaleImpl;

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
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, long int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, long long int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned short int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned long int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, unsigned long long int data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
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
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteStringBuilder &operator<<(ByteStringBuilder &b, double data)
{
	LocaleImpl.PrepareStream(b);
	LocaleImpl.stream << data;
	LocaleImpl.FlushStream(b);
	return b;
}

ByteString::Split ByteString::SplitSigned(long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareStream(*this, pos, set, reset);
	LocaleImpl.stream >> value;
	if(LocaleImpl.stream.fail())
	{
		LocaleImpl.FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.stream.clear();
	Split split(*this, pos, pos + LocaleImpl.stream.tellg(), 0, false);
	LocaleImpl.FlushStream();
	return split;
}

ByteString::Split ByteString::SplitUnsigned(unsigned long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareStream(*this, pos, set, reset);
	LocaleImpl.stream >> value;
	if(LocaleImpl.stream.fail())
	{
		LocaleImpl.FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.stream.clear();
	Split split(*this, pos, pos + LocaleImpl.stream.tellg(), 0, false);
	LocaleImpl.FlushStream();
	return split;
}

ByteString::Split ByteString::SplitFloat(double &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareStream(*this, pos, set, reset);
	LocaleImpl.stream >> value;
	if(LocaleImpl.stream.fail())
	{
		LocaleImpl.FlushStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.stream.clear();
	Split split(*this, pos, pos + LocaleImpl.stream.tellg(), 0, false);
	LocaleImpl.FlushStream();
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
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, long int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, long long int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned short int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned long int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, unsigned long long int data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
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
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

StringBuilder &operator<<(StringBuilder &b, double data)
{
	LocaleImpl.PrepareWStream(b);
	LocaleImpl.wstream << data;
	LocaleImpl.FlushWStream(b);
	return b;
}

String::Split String::SplitSigned(long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareWStream(*this, pos, set, reset);
	LocaleImpl.wstream >> value;
	if(LocaleImpl.wstream.fail())
	{
		LocaleImpl.FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.wstream.clear();
	Split split(*this, pos, pos + LocaleImpl.wstream.tellg(), 0, false);
	LocaleImpl.FlushWStream();
	return split;
}

String::Split String::SplitUnsigned(unsigned long long int &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareWStream(*this, pos, set, reset);
	LocaleImpl.wstream >> value;
	if(LocaleImpl.wstream.fail())
	{
		LocaleImpl.FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.wstream.clear();
	Split split(*this, pos, pos + LocaleImpl.wstream.tellg(), 0, false);
	LocaleImpl.FlushWStream();
	return split;
}

String::Split String::SplitFloat(double &value, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset) const
{
	LocaleImpl.PrepareWStream(*this, pos, set, reset);
	LocaleImpl.wstream >> value;
	if(LocaleImpl.wstream.fail())
	{
		LocaleImpl.FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.wstream.clear();
	Split split(*this, pos, pos + LocaleImpl.wstream.tellg(), 0, false);
	LocaleImpl.FlushWStream();
	return split;
}
