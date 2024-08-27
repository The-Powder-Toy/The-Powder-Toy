#pragma once

namespace http
{
	enum Category
	{
		categoryNone,
		categoryMyOwn,
		categoryFavourites,
	};

	enum Sort
	{
		sortByVotes,
		sortByDate,
	};

	enum Period
	{
		allSaves,
		todaySaves,
		weekSaves,
		monthSaves,
		yearSaves,
	};
}
