#pragma once
#include "common/String.h"
#include <json/json.h>
#include <optional>

class Prefs
{
	struct DeferWriteTag
	{
	};

	Json::Value root;
	static Json::Value GetJson(const Json::Value &node, ByteString path);
	static void SetJson(Json::Value &node, ByteString path, Json::Value value);

	template<class Type>
	struct Bipacker
	{
		static Json::Value Pack(const Type &value);
		static Type Unpack(const Json::Value &value);
	};

	void Read();
	void Write();
	void ShouldWrite();
	unsigned int deferWriteLevel = 0;
	bool backedByFile = false;
	bool shouldWrite = false;

	ByteString path;

	Prefs(const Prefs &) = delete;
	Prefs &operator =(const Prefs &) = delete;

public:
	Prefs(ByteString path);

	template<class Type>
	std::optional<Type> Get(ByteString path) const
	{
		auto value = GetJson(root, path);
		if (value != Json::nullValue)
		{
			try
			{
				return Bipacker<Type>::Unpack(value);
			}
			catch (const std::exception &e)
			{
			}
		}
		return std::nullopt;
	}

	template<class Type>
	Type Get(ByteString path, Type defaultValue) const
	{
		auto value = Get<Type>(path);
		if (value)
		{
			return *value;
		}
		return defaultValue;
	}

	template<class Enum, class EnumBase = int>
	Enum Get(ByteString path, Enum maxValue, Enum defaultValue) const
	{
		EnumBase value = Get(path, EnumBase(defaultValue));
		if (value < 0 || value >= EnumBase(maxValue))
		{
			value = EnumBase(defaultValue);
		}
		return Enum(value);
	}

	template<class Type>
	void Set(ByteString path, Type value)
	{
		SetJson(root, path, Bipacker<Type>::Pack(value));
		ShouldWrite();
	}

	void Clear(ByteString path)
	{
		SetJson(root, path, Json::nullValue);
		ShouldWrite();
	}

	void GrabDeferWriteLevel(DeferWriteTag);
	void DropDeferWriteLevel(DeferWriteTag);

	struct DeferWrite
	{
		Prefs &prefs;

		DeferWrite(const DeferWrite &) = delete;
		DeferWrite &operator =(const DeferWrite &) = delete;

		DeferWrite(Prefs &newPrefs) : prefs(newPrefs)
		{
			prefs.GrabDeferWriteLevel({});
		}

		~DeferWrite()
		{
			prefs.DropDeferWriteLevel({});
		}
	};

	bool BackedByFile() const
	{
		return backedByFile;
	}
};
