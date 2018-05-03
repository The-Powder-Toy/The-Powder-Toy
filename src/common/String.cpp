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

String numberChars = "-.+0123456789ABCDEFXabcdefx";

static thread_local struct LocaleImpl
{
	std::basic_stringstream<char> stream;
	std::basic_stringstream<wchar_t> wstream;

	LocaleImpl()
	{
		stream.imbue(std::locale::classic());
		wstream.imbue(std::locale::classic());
	}

	inline void PrepareWStream(StringBuilder &b)
	{
		wstream.flags(b.flags);
		wstream.width(b.width);
		wstream.precision(b.precision);
		wstream.fill(b.fill);
	}

	inline void PrepareWStream(String const &str, size_t pos, std::ios_base::fmtflags set, std::ios_base::fmtflags reset)
	{
		wstream.flags((std::ios_base::dec & ~ reset) | set);
		std::basic_string<wchar_t> wstr;
		while(pos < str.size() && representable_wchar(str[pos]) && numberChars.Contains(str[pos]))
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
		LocaleImpl.wstream.clear();
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
		LocaleImpl.wstream.clear();
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
		LocaleImpl.wstream.clear();
		LocaleImpl.FlushWStream();
		return Split(*this, pos, npos, 0, false);
	}
	LocaleImpl.wstream.clear();
	Split split(*this, pos, pos + LocaleImpl.wstream.tellg(), 0, false);
	LocaleImpl.FlushWStream();
	return split;
}
