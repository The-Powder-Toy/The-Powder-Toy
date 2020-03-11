#pragma once

#include <array>
#include <map>

#include "String.h"

/*
	We handle internationalization by maintaining a map from "key" strings, to
	localized versions of those strings. The "keys" are strings in English, the
	default language. At application startup this map is populated by
	translations based on current settings. In theory this map could be updated
	on the fly at runtime but various GUI interfaces will "cache" translations
	of the strings that they use and working around this is tricky, thus we
	require an application restart to change the language.

	For performance reasons we rely on the assumption that we only need to
	translate strings that are actual compile-time string literal constants.
	Such literals are very easily equated by their pointer value: a pointer to a
	string literal will always point to that literal and nothing else. The
	LiteralPtr type is a pointer with this assumption.

	However two instances of the same string literal might not have the same
	pointer (in particular if the literal appears in different compilation
	units; albeit depends on the compiler and compiler options). We thus
	introduce another layer of indirection: a map from string literal pointers
	to a "canonical" pointer for that string literal. Really it's just an
	arbitrarily chosen literal pointer for that string. Invariant: given
	CanonicalPtr c, LiteralPtr l, if !strcmp(l, c) then Canonicalize(l) == c.

	Sometimes several pieces of strings are assembled into a larger string,
	possibly with data inbetween, e.g.:
		String::Build("Page ", page, " of ", total)
	In the C world we could've gotten away with using a key with placeholders 
	such as "Page %d of %d" and then the translation would include the format
	specifiers as well and it would all work out. Note that it is a bad idea to
	introduce "Page" and "of" as separate keys as they don't make much sense on
	their own. Our solution is to allow keys to be a *sequence* of English
	strings that are intended to be used together, and the translation is a
	sequence of the same length. In the above example our key would be
	{"Page ", " of "}. We will then obtain a translation which would be a pair
	of strings that could be fed into String::Build.

	Sometimes the same English string is used in multiple places in different
	contexts, and in a different language the different uses might require
	different translations, e.g. "Login" could be a verb, or a noun. To remedy
	this we can add a string to the key that would indicate the context, even
	though the context part is never shown to the user, e.g.
	{"Login", "Authenticate"} vs {"Login", "Username"}.

	This interface boils down to a user-defined string literal operator""_i18n
	that lets you write "Foo"_i18n which would look up translation for the key
	"Foo" at runtime; and the function i18nMulti(...) which takes multiple keys
	and returns an array of the same size with the translation of the entire
	sequence.
*/

namespace i18n
{
	using LiteralPtr = char const *;
	using CanonicalPtr = char const *;

	CanonicalPtr Canonicalize(LiteralPtr str);

	template<size_t n> struct TranslationMap
	{
		// This is not just a static field so that it is possible to use it
		// during dynamic initialization of global variables
		static std::map<std::array<CanonicalPtr, n>, std::array<String, n>> &Map()
		{
			static std::map<std::array<CanonicalPtr, n>, std::array<String, n>> map{};
			return map;
		}
	};

	template<size_t N> String &translation(char const (&str)[N])
	{
		return TranslationMap<1>::Map()[{Canonicalize(str)}][0];
	}
	
	template<size_t n> inline std::array<String, n> &multiTranslation(std::array<CanonicalPtr, n> &cans, size_t)
	{
		return TranslationMap<n>::Map()[cans];
	}

	template<size_t n, size_t N, typename... Ts> std::array<String, n> &multiTranslation(std::array<CanonicalPtr, n> &cans, size_t i, char const (&lit)[N], Ts&&... args)
	{
		cans[i] = Canonicalize(lit);
		return multiTranslation(cans, i + 1, std::forward<Ts>(args)...);
	}

	template<typename... Ts> std::array<String, sizeof...(Ts)> &translation(Ts&&... args)
	{
		std::array<CanonicalPtr, sizeof...(Ts)> strs;
		return multiTranslation(strs, 0, std::forward<Ts>(args)...);
	}

	template<size_t n> inline std::array<String, n> getTranslation(std::array<LiteralPtr, n> strs)
	{
		std::array<CanonicalPtr, n> cans;
		for(size_t i = 0; i < n; i++)
			cans[i] = Canonicalize(strs[i]);
		auto it = TranslationMap<n>::Map().find(cans);
		if(it != TranslationMap<n>::Map().end())
			return it->second;
		std::array<String, n> defs;
		for(size_t i = 0; i < n; i++)
			defs[i] = ByteString(strs[i]).FromAscii();
		return defs;
	}

	template<size_t n> inline std::array<String, n> getMultiTranslation(std::array<LiteralPtr, n> &strs, size_t)
	{
		return getTranslation<n>(strs);
	}

	template<size_t n, size_t N, typename... Ts> std::array<String, n> getMultiTranslation(std::array<LiteralPtr, n> &strs, size_t i, char const (&lit)[N], Ts&&... args)
	{
		strs[i] = lit;
		return getMultiTranslation(strs, i + 1, std::forward<Ts>(args)...);
	}
}

inline String operator""_i18n(char const *str, size_t)
{
	return i18n::getTranslation<1>({str})[0];
}

template<typename... Ts> std::array<String, sizeof...(Ts)> i18nMulti(Ts&&... args)
{
	std::array<i18n::LiteralPtr, sizeof...(Ts)> strs;
	return i18n::getMultiTranslation(strs, 0, std::forward<Ts>(args)...);
}
