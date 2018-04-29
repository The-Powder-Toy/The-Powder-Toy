#ifndef TPT_STRING
#define TPT_STRING

#include <string>

class String;

class ByteString : public std::basic_string<char>
{
public:
	inline ByteString(): std::basic_string<char>() {}
	inline ByteString(size_type count, value_type ch): std::basic_string<char>(count, ch) {}
	inline ByteString(value_type const *ch, size_type count): std::basic_string<char>(ch, count) {}
	inline ByteString(value_type const *ch): std::basic_string<char>(ch) {}
	template<class It> inline ByteString(It first, It last): std::basic_string<char>(first, last) {}
	inline ByteString(ByteString const &other): std::basic_string<char>(other) {}
	inline ByteString(ByteString &&other): std::basic_string<char>(std::move(other)) {}
	
	ByteString &operator=(ByteString const &other) { std::basic_string<char>::operator=(other); return *this; }
	ByteString &operator=(ByteString &&other) { std::basic_string<char>::operator=(std::move(other)); return *this; }

	class ConversionError : public std::runtime_error
	{
		static std::string formatError(value_type const *at, value_type const *upto);
	public:
		inline ConversionError(value_type const *at, value_type const *upto): std::runtime_error(formatError(at, upto)) {}
		inline ConversionError(bool to): std::runtime_error(to ? "Could not convert to UTF-8" : "Could not convert from UTF-8") {}
	};

	String FromUtf8(bool ignoreError = true) const;
	inline String FromAscii() const;
};

class String : public std::basic_string<char32_t>
{
public:
	inline String(): std::basic_string<char32_t>() {}
	inline String(size_type count, value_type ch): std::basic_string<char32_t>(count, ch) {}
	inline String(value_type const *ch, size_type count): std::basic_string<char32_t>(ch, count) {}
	inline String(value_type const *ch): std::basic_string<char32_t>(ch) {}
	template<class It> inline String(It first, It last): std::basic_string<char32_t>(first, last) {}
	inline String(String const &other): std::basic_string<char32_t>(other) {}
	inline String(String &&other): std::basic_string<char32_t>(std::move(other)) {}

	String &operator=(String const &other) { std::basic_string<char32_t>::operator=(other); return *this; }
	String &operator=(String &&other) { std::basic_string<char32_t>::operator=(std::move(other)); return *this; }

	template<unsigned N> inline String(ByteString::value_type const (&ch)[N]): std::basic_string<char32_t>(ByteString(ch, N).FromAscii()) {}

	ByteString ToUtf8() const;
};

inline String ByteString::FromAscii() const
{
	String destination = String(size(), String::value_type());
	for(size_t i = 0; i < size(); i++)
		destination[i] = typename String::value_type(operator[](i));
	return destination;
}
#endif
