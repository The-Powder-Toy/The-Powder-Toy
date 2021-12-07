#include "GOLString.h"
#include "client/Client.h"

int ParseGOLString(const String &value)
{
	// * Most likely a GOL string.
	auto it = value.begin() + 1;
	auto begin = 0U;
	auto stay = 0U;
	auto states = 2U;

	// Scan 'B' section, must be between 1 and 8
	for (; it != value.end() && it[0] >= '1' && it[0] <= '8'; ++it)
	{
		begin |= 1U << (it[0] - '0');
	}
	if (!begin)
	{
		return -1;
	}

	// Must have a /S immediately afterwards
	if (it < value.end() - 1 && it[0] == '/' && it[1] == 'S')
	{
		it += 2;
	}
	else
	{
		return -1;
	}

	// Scan 'S' section, must be between 0 and 8
	for (; it != value.end() && it[0] >= '0' && it[0] <= '8'; ++it)
	{
		stay |= 1U << (it[0] - '0');
	}

	// Optionally can have a 3rd section, with the number of frames to remain after dying
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
		states = String(it, value.end()).ToNumber<unsigned int>(true);
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
		if (!((ch >= '0' && ch < '9') || (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '-')))
		{
			nameOk = false;
		}
	}
	return nameOk;
}

String SerialiseGOLRule(int rule)
{
	StringBuilder golName;
	golName << "B";
	for (int i = 1; i < 9; ++i)
	{
		if ((rule >> (i + 8)) & 1)
		{
			golName << char('0' + i);
		}
	}
	golName << "/S";
	for (int i = 0; i < 9; ++i)
	{
		if ((rule >> i) & 1)
		{
			golName << char('0' + i);
		}
	}
	if ((rule >> 17) & 0xF)
	{
		golName << "/" << ((rule >> 17) & 0xF) + 2;
	}
	return golName.Build();
}

#ifndef RENDERER
bool AddCustomGol(String ruleString, String nameString, unsigned int highColor, unsigned int lowColor)
{
	auto customGOLTypes = Client::Ref().GetPrefByteStringArray("CustomGOL.Types");
	Json::Value newCustomGOLTypes(Json::arrayValue);
	bool nameTaken = false;
	for (auto gol : customGOLTypes)
	{
		auto parts = gol.FromUtf8().PartitionBy(' ');
		if (parts.size())
		{
			if (parts[0] == nameString)
			{
				nameTaken = true;
			}
		}
		newCustomGOLTypes.append(gol);
	}
	if (nameTaken)
		return false;

	StringBuilder sb;
	sb << nameString << " " << ruleString << " " << highColor << " " << lowColor;
	newCustomGOLTypes.append(sb.Build().ToUtf8());
	Client::Ref().SetPref("CustomGOL.Types", newCustomGOLTypes);
	return true;
}
#endif
