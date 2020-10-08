#include "GOLString.h"

int ParseGOLString(const String &value)
{
	// * Most likely a GOL string.
	auto it = value.begin() + 1;
	auto begin = 0U;
	auto stay = 0U;
	auto states = 2U;

	for (; it != value.end() && it[0] >= '1' && it[0] <= '8'; ++it)
	{
		begin |= 1U << (it[0] - '0');
	}

	if (it < value.end() - 1 && it[0] == '/' && it[1] == 'S')
	{
		it += 2;
	}
	else
	{
		return -1;
	}

	for (; it != value.end() && it[0] >= '1' && it[0] <= '8'; ++it)
	{
		stay |= 1U << (it[0] - '0');
	}

	if (it != value.end())
	{
		if (it[0] == '/')
		{
			it += 1;
		}
		else
		{
			return -1;
		}
		states = String(it, value.end()).ToNumber<unsigned int>();
		if (states < 2 || states > 17)
		{
			return -1;
		}
	}

	return stay | (begin << 8) | ((states - 2) << 17);
}

bool ValidateGOLName(const String &value)
{
	bool nameOk = true;
	for (auto ch : value)
	{
		if (!((ch >= '0' && ch < '9') || (ch >= 'A' && ch <= 'Z') || (ch == '-')))
		{
			nameOk = false;
		}
	}
	return nameOk;
}
