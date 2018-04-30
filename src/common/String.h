#ifndef TPT_STRING
#define TPT_STRING

#include <stdexcept>
#include <sstream>
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
	inline ByteString(std::basic_string<char> const &other): std::basic_string<char>(other) {}
	inline ByteString(std::basic_string<char> &&other): std::basic_string<char>(std::move(other)) {}
	inline ByteString(ByteString const &other): std::basic_string<char>(other) {}
	inline ByteString(ByteString &&other): std::basic_string<char>(std::move(other)) {}
	
	inline ByteString &operator=(ByteString const &other) { std::basic_string<char>::operator=(other); return *this; }
	inline ByteString &operator=(ByteString &&other) { std::basic_string<char>::operator=(std::move(other)); return *this; }
	
	template<typename T> ByteString &operator+=(T &&other) { std::basic_string<char>::operator+=(std::forward<T>(other)); return *this; }
	template<typename T> inline ByteString operator+(T &&other) const { ByteString tmp = *this; tmp += std::forward<T>(other); return tmp; }
	template<typename... Ts> ByteString substr(Ts&&... args) const { return std::basic_string<char>::substr(std::forward<Ts>(args)...); }
	template<typename... Ts> ByteString &insert(Ts&&... args) { std::basic_string<char>::insert(std::forward<Ts>(args)...); return *this; }

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

inline ByteString operator+(ByteString::value_type const *ch, ByteString const &str) { return ByteString(ch) + str; }
inline ByteString operator+(std::basic_string<char> const &other, ByteString const &str) { return ByteString(other) + str; }
inline ByteString operator+(std::basic_string<char> &&other, ByteString const &str) { return ByteString(std::move(other)) + str; }

class String : public std::basic_string<char32_t>
{
public:
	inline String(): std::basic_string<char32_t>() {}
	inline String(size_type count, value_type ch): std::basic_string<char32_t>(count, ch) {}
	inline String(value_type const *ch, size_type count): std::basic_string<char32_t>(ch, count) {}
	inline String(value_type const *ch): std::basic_string<char32_t>(ch) {}
	template<class It> inline String(It first, It last): std::basic_string<char32_t>(first, last) {}
	inline String(std::basic_string<char32_t> const &other): std::basic_string<char32_t>(other) {}
	inline String(std::basic_string<char32_t> &&other): std::basic_string<char32_t>(std::move(other)) {}
	inline String(String const &other): std::basic_string<char32_t>(other) {}
	inline String(String &&other): std::basic_string<char32_t>(std::move(other)) {}
	template<unsigned N> inline String(ByteString::value_type const (&ch)[N]): std::basic_string<char32_t>(ByteString(ch, N - 1).FromAscii()) {}

	inline String &operator=(String const &other) { std::basic_string<char32_t>::operator=(other); return *this; }
	inline String &operator=(String &&other) { std::basic_string<char32_t>::operator=(other); return *this; }
	
	template<typename T> inline String &operator+=(T &&other) { std::basic_string<char32_t>::operator+=(std::forward<T>(other)); return *this; }
	template<unsigned N> inline String &operator+=(ByteString::value_type const (&ch)[N]) { std::basic_string<char32_t>::operator+=(ByteString(ch, N - 1).FromAscii()); return *this; }
	template<typename T> inline String operator+(T &&other) const { String tmp = *this; tmp += std::forward<T>(other); return tmp; }
	template<typename... Ts> inline String substr(Ts&&... args) const { return std::basic_string<char32_t>::substr(std::forward<Ts>(args)...); }
	inline String &insert(size_t pos, String &str) { std::basic_string<char32_t>::insert(pos, str); return *this; }
	inline String &insert(size_t pos, size_t n, value_type ch) { std::basic_string<char32_t>::insert(pos, n, ch); return *this; }
	template<unsigned N> inline String &insert(size_t pos, ByteString::value_type const (&ch)[N]) { std::basic_string<char32_t>::insert(pos, ByteString(ch, N - 1).FromAscii()); return *this; }
	inline size_t find(String const &str, size_t pos = 0) { return std::basic_string<char32_t>::find(str, pos); }
	inline size_t find(value_type ch, size_t pos = 0) { return std::basic_string<char32_t>::find(ch, pos); }

	inline bool operator==(String const &other) { return std::basic_string<char32_t>(*this) == other; }

	ByteString ToUtf8() const;
	ByteString ToAscii() const;

	using Stream = std::basic_stringstream<value_type>;
};

inline String operator+(String::value_type const *ch, String const &str) { return String(ch) + str; }
inline String operator+(std::basic_string<char32_t> const &other, String const &str) { return String(other) + str; }
inline String operator+(std::basic_string<char32_t> &&other, String const &str) { return String(std::move(other)) + str; }
template<unsigned N> inline String operator+(ByteString::value_type const (&ch)[N], String const &str) { return String(ch) + str; }


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
