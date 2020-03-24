#pragma once

#include <functional>

struct Locale
{
	// The name of the language this locale is for, readable in both the native
	// language and in English;
	virtual class String GetName() const = 0;

	// Populate the translations map.
	virtual void Set() const = 0;

	virtual class String GetIntroText() const = 0;
	virtual class String GetSavePublishingInfo() const = 0;
	virtual class String GetRules() const = 0;

	/*
		English only has two choices for spelling a noun based on amount:
		singular when there's exactly 1 object, and plural when there's 0 or 2
		or more objects.

		In Russian there are three choices, based on the last 2 digits of the
		amount: singular nominative if the number ends in 1, except 11;
		singular genitive if the number ends in 2,3,4, except 12 through 14;
		plural genitive if the for everything else.

		In other languages things can get more complicated, for example in
		Arabic a noun can have as many as 6 spellings.

		What we can do is give these various forms indices, e.g. for Russian
		singular nominative=0, singular genitive=1, plural genitive=2;
		and then use these indices to index into an array of spellings for the
		respective word.

		GetPluralIndex(n) returns the index of the word form used for n objects.
	*/
	virtual size_t GetPluralIndex(size_t n) const = 0;
};
