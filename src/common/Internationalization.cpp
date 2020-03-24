#include "Internationalization.h"

namespace i18n
{
	// These are not global variables so that it is possible to use them during
	// dynamic initialization of other global variables
	static std::map<LiteralPtr, CanonicalPtr> &literalCanonicalization()
	{
		static std::map<LiteralPtr, CanonicalPtr> literalCanonicalization{};
		return literalCanonicalization;
	}

	static std::map<ByteString, CanonicalPtr> &canonicalization()
	{
		static std::map<ByteString, CanonicalPtr> canonicalization{};
		return canonicalization;
	}

	CanonicalPtr Canonicalize(LiteralPtr str)
	{
		auto it = literalCanonicalization().find(str);
		if(it == literalCanonicalization().end())
		{
			CanonicalPtr can = canonicalization().insert(std::make_pair(str, str)).first->second;
			literalCanonicalization().insert(std::make_pair(str, can));
			return can;
		}
		else
			return it->second;
	}

#ifdef I18N_DEBUG
    std::set<std::vector<ByteString> > &activeKeys()
	{
		static std::set<std::vector<ByteString> > activeKeys{};
		return activeKeys;
	}

	std::set<ByteString> &activePlurals()
	{
		static std::set<ByteString> activePlurals{};
		return activePlurals;
	}
#endif // I18N_DEBUG
}

Locale const *currentLocale = nullptr;
