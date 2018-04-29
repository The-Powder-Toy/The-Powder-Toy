#include <sstream>
#include <vector>
#include <locale>
#include <codecvt>

#include "String.h"

std::string ByteString::ConversionError::formatError(ByteString::value_type const *at, ByteString::value_type const *upto)
{
	std::stringstream ss;
	ss << "Could not convert sequence to UTF-8:";
	for(int i = 0; i < 4 && at + i < upto; i++)
		ss << " " << std::hex << (unsigned int)std::make_unsigned<ByteString::value_type>::type(at[i]);
	return ss.str();
}


std::codecvt_utf8<char32_t> convert(1);

String ByteString::FromUtf8(bool ignoreError) const
{
	std::vector<String::value_type> destination = std::vector<String::value_type>(size(), String::value_type());
	std::codecvt_utf8<char32_t>::state_type state;

	ByteString::value_type const *from = data(), *from_next;
	String::value_type *to = destination.data(), *to_next;

	while(true)
	{
		std::codecvt_utf8<char32_t>::result result = convert.in(state, from, data() + size(), from_next, to, destination.data() + destination.size(), to_next);
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

ByteString String::ToUtf8() const
{
	std::vector<ByteString::value_type> destination = std::vector<ByteString::value_type>(size(), ByteString::value_type());
	std::codecvt_utf8<char32_t>::state_type state;

	String::value_type const *from = data(), *from_next;
	ByteString::value_type *to = destination.data(), *to_next;

	while(true)
	{
		std::codecvt_utf8<char32_t>::result result = convert.out(state, from, data() + size(), from_next, to, destination.data() + destination.size(), to_next);
		from = from_next;
		to = to_next;
		if(result == std::codecvt_base::ok || result == std::codecvt_base::noconv)
		{
			destination.resize(to - destination.data());
			return ByteString(destination.data(), destination.size());
		}
		else if(result == std::codecvt_base::error)
		{
			throw ByteString::ConversionError(true);
		}
		else if(result == std::codecvt_base::partial)
		{
			ByteString::value_type *old_data = destination.data();
			destination.resize(2 * destination.size());
			to = destination.data() + (to - old_data);
		}
	}
}
