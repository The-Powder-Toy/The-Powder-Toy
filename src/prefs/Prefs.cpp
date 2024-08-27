#include "Prefs.h"
#include "common/platform/Platform.h"
#include "common/tpt-rand.h"
#include "client/User.h"
#include <fstream>
#include <iostream>

Prefs::Prefs(ByteString newPath) : path(newPath)
{
	Read();
}

void Prefs::Read()
{
	std::vector<char> data;
	if (!Platform::ReadFile(data, path))
	{
		return;
	}
	Json::CharReaderBuilder rbuilder;
	std::unique_ptr<Json::CharReader> const reader(rbuilder.newCharReader());
	ByteString errs;
	if (!data.size())
	{
		std::cerr << "no json data" << std::endl;
		return;
	}
	if (!reader->parse(data.data(), data.data() + data.size(), &root, &errs))
	{
		std::cerr << errs << std::endl;
		return;
	}
	backedByFile = true;
}

void Prefs::ShouldWrite()
{
	shouldWrite = true;
	Write();
}

void Prefs::Write()
{
	if (deferWriteLevel)
	{
		return;
	}
	if (!shouldWrite)
	{
		return;
	}
	shouldWrite = false;
	Json::StreamWriterBuilder wbuilder;
	wbuilder["indentation"] = "\t";
	ByteString data = Json::writeString(wbuilder, root);
	if (!Platform::WriteFile(std::vector<char>(data.begin(), data.end()), path))
	{
		return;
	}
	backedByFile = true;
}

void Prefs::GrabDeferWriteLevel(DeferWriteTag)
{
	deferWriteLevel += 1;
}

void Prefs::DropDeferWriteLevel(DeferWriteTag)
{
	deferWriteLevel -= 1;
	Write();
}

Json::Value Prefs::GetJson(const Json::Value &node, ByteString path)
{
	if (node.type() != Json::objectValue)
	{
		return Json::nullValue;
	}
	auto split = path.SplitBy('.');
	if (!split)
	{
		return node[path];
	}
	return GetJson(node[split.Before()], split.After());
}

void Prefs::SetJson(Json::Value &node, ByteString path, Json::Value value)
{
	if (node.type() != Json::objectValue)
	{
		node = Json::objectValue;
	}
	auto split = path.SplitBy('.');
	if (!split)
	{
		node[path] = value;
		return;
	}
	SetJson(node[split.Before()], split.After(), value);
}

template<> Json::Value Prefs::Bipacker<int>::Pack  (const int         &value) { return Json::Value(value); }
template<> int         Prefs::Bipacker<int>::Unpack(const Json::Value &value) { return value.asInt(); }

template<> Json::Value  Prefs::Bipacker<unsigned int>::Pack  (const unsigned int &value) { return Json::Value(value); }
template<> unsigned int Prefs::Bipacker<unsigned int>::Unpack(const Json::Value  &value) { return value.asUInt(); }

template<> Json::Value Prefs::Bipacker<uint64_t>::Pack  (const uint64_t    &value) { return Json::Value(Json::UInt64(value)); }
template<> uint64_t    Prefs::Bipacker<uint64_t>::Unpack(const Json::Value &value) { return value.asUInt64(); }

template<> Json::Value Prefs::Bipacker<float>::Pack  (const float       &value) { return Json::Value(value); }
template<> float       Prefs::Bipacker<float>::Unpack(const Json::Value &value) { return value.asFloat(); }

template<> Json::Value Prefs::Bipacker<bool>::Pack  (const bool        &value) { return Json::Value(value); }
template<> bool        Prefs::Bipacker<bool>::Unpack(const Json::Value &value) { return value.asBool(); }

template<> Json::Value Prefs::Bipacker<ByteString>::Pack  (const ByteString  &value) { return Json::Value(value); }
template<> ByteString  Prefs::Bipacker<ByteString>::Unpack(const Json::Value &value) { return value.asString(); }

template<> Json::Value Prefs::Bipacker<String>::Pack  (const String      &value) { return Json::Value(value.ToUtf8()); }
template<> String      Prefs::Bipacker<String>::Unpack(const Json::Value &value) { return ByteString(value.asString()).FromUtf8(); }

template<> Json::Value     Prefs::Bipacker<User::Elevation>::Pack  (const User::Elevation &value) { return Json::Value(User::ElevationToString(value)); }
template<> User::Elevation Prefs::Bipacker<User::Elevation>::Unpack(const Json::Value     &value) { return User::ElevationFromString(value.asString()); }

template<class Item>
struct Prefs::Bipacker<std::vector<Item>>
{
	static Json::Value Pack(const std::vector<Item> &value);
	static std::vector<Item> Unpack(const Json::Value &value);
};

template<class Item>
Json::Value Prefs::Bipacker<std::vector<Item>>::Pack(const std::vector<Item> &value)
{
	Json::Value array = Json::arrayValue;
	for (auto item : value)
	{
		array.append(Bipacker<Item>::Pack(item));
	}
	return array;
}

template<class Item>
std::vector<Item> Prefs::Bipacker<std::vector<Item>>::Unpack(const Json::Value &value)
{
	std::vector<Item> array;
	if (value.type() != Json::arrayValue)
	{
		throw std::exception();
	}
	for (auto &item : value)
	{
		array.push_back(Bipacker<Item>::Unpack(item));
	}
	return array;
}

template struct Prefs::Bipacker<std::vector<String>>;
template struct Prefs::Bipacker<std::vector<ByteString>>;
template struct Prefs::Bipacker<std::vector<unsigned int>>;
