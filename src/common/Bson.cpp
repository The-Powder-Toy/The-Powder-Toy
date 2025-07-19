#include "Bson.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstring>
#include <stack>

namespace
{
	constexpr int32_t maxDepth = 100;
	constexpr int32_t maxNodes = 1000000;

	struct Reader
	{
		int32_t at = 0;
		std::span<const char> data;

		int32_t GetLeft() const
		{
			return int32_t(data.size());
		}

		void Advance(int32_t by)
		{
			at += by;
			data = data.subspan(size_t(by));
		}

		void Throw(ByteString what) const
		{
			throw Bson::ParseError(ByteString::Build(what, " at byte offset ", at));
		}

		void Bound(const char *what, int32_t size) const
		{
			if (GetLeft() < size)
			{
				Throw(ByteString::Build("missing ", what));
			}
		}

		template<class Type>
		Type Read(const char *what)
		{
			Bound(what, int32_t(sizeof(Type)));
			std::array<char, sizeof(Type)> value;
			std::memcpy(value.data(), data.data(), sizeof(Type));
			if constexpr (std::endian::native == std::endian::big)
			{
				std::reverse(value.begin(), value.end());
			}
			return std::bit_cast<Type>(value);
		}

		template<class Type>
		Type Take(const char *what)
		{
			auto value = Read<Type>(what);
			Advance(sizeof(Type));
			return value;
		}
	};

	struct Writer
	{
		std::vector<char> data;

		template<class Type>
		void WriteAt(int32_t at, const Type &thing)
		{
			auto value = std::bit_cast<std::array<char, sizeof(Type)>>(thing);
			if constexpr (std::endian::native == std::endian::big)
			{
				std::reverse(value.begin(), value.end());
			}
			std::copy(value.begin(), value.end(), data.begin() + at);
		}

		template<class Type>
		void Write(const Type &thing)
		{
			auto oldSize = data.size();
			auto newSize = oldSize + sizeof(Type);
			if (newSize > INT32_MAX)
			{
				throw Bson::DumpError("output too big");
			}
			data.resize(newSize);
			WriteAt(int32_t(oldSize), thing);
		}

		int32_t GetHave() const
		{
			return int32_t(data.size());
		}
	};
}

Bson Bson::Parse(std::span<const char> data, const Bson *rootNonconformance)
{
	Bson root(Type::objectValue);
	struct StackEntry
	{
		Bson *value;
		const Bson *nonconformance;
		Reader reader;
	};
	std::stack<StackEntry> stack;
	{
		Reader reader{ 0, data };
		auto size = reader.Read<int32_t>("root size");
		auto subreader = reader;
		reader.Bound("root content", size);
		subreader.Advance(sizeof(int32_t));
		stack.push({ &root, rootNonconformance, subreader });
	}
	int32_t nodes = 0;
	while (!stack.empty())
	{
		auto &top = stack.top();
		auto &reader = top.reader;
		nodes += 1;
		if (nodes >= maxNodes)
		{
			reader.Throw("max node count exceeded");
		}
		auto elementReader = reader;
		auto elementType = int32_t(reader.Take<int8_t>("element type"));
		if (elementType == 0)
		{
			stack.pop();
			continue;
		}
		String elementName;
		{
			auto elementNameReader = reader;
			auto beginAt = reader.at;
			while (reader.Take<uint8_t>("element key byte"));
			elementName = String(elementNameReader.data.data(), reader.at - 1 - beginAt);
		}
		auto push = [&](Bson element) -> Bson & {
			auto &value = *top.value;
			if (value.Is<Array>())
			{
				return value.Append(std::move(element));
			}
			return value[elementName] = std::move(element);
		};
		switch (elementType)
		{
		case 3:
		case 4:
			{
				const Bson *nonconformance = nullptr;
				if (top.nonconformance && top.nonconformance->Is<Object>())
				{
					nonconformance = top.nonconformance->Get(elementName);
				}
				if (nonconformance && nonconformance->Is<Array>() && nonconformance->Get(0)->As<String>() == "objectEncodedAsArray")
				{
					elementType = 3;
				}
				auto &document = push(elementType == 3 ? Type::objectValue : Type::arrayValue);
				auto size = reader.Read<int32_t>("document size");
				auto subreader = reader;
				reader.Bound("document content", size);
				reader.Advance(size);
				subreader.Advance(sizeof(int32_t));
				stack.push({ &document, nonconformance, subreader });
				if (int32_t(stack.size()) >= maxDepth)
				{
					reader.Throw("max depth exceeded");
				}
			}
			break;

		case 2:
			{
				auto sizeReader = reader;
				auto size = reader.Take<int32_t>("string size");
				reader.Bound("string content", size);
				if (size < 1)
				{
					sizeReader.Throw("invalid size for string");
				}
				push(String(reader.data.data(), reader.data.data() + size - 1));
				reader.Advance(size);
			}
			break;

		case 5:
			{
				auto size = reader.Take<int32_t>("userdata size");
				reader.Bound("userdata content", size);
				auto subtypeReader = reader;
				auto subtype = int32_t(reader.Take<uint8_t>("userdata subtype"));
				if (subtype != 128)
				{
					subtypeReader.Throw(ByteString::Build("bad userdata subtype ", subtype));
				}
				auto &user = push(User(size)).As<User>();
				std::copy_n(reader.data.data(), size, user.begin());
				reader.Advance(size);
			}
			break;

		case  1: push(reader.Take<double>("double value"));      break;
		case  8: push(bool(reader.Take<uint8_t>("bool value"))); break;
		case 10:                                                 break;
		case 16: push(reader.Take<int32_t>("int32 value"));      break;
		case 18: push(reader.Take<int64_t>("int64 value"));      break;

		default:
			elementReader.Throw(ByteString::Build("bad element type ", elementType));
			break;
		}
	}
	return root;
}

std::vector<char> Bson::Dump(const Bson *rootNonconformance) const
{
	assert(Is<Object>());
	struct StackEntry
	{
		const Bson *value;
		const Bson *nonconformance;
		int32_t patchSizeAt;
		int32_t childIndex;
		Object::const_iterator childIterator;
	};
	Writer writer;
	std::stack<StackEntry> stack;
	{
		auto sizeAt = writer.GetHave();
		writer.Write(int32_t(0));
		stack.push({ this, rootNonconformance, sizeAt, 0, As<Object>().begin() });
	}
	while (!stack.empty())
	{
		auto &top = stack.top();
		auto &value = *top.value;
		auto *nonconformance = top.nonconformance;
		if (top.childIndex >= int32_t(value.GetSize()))
		{
			writer.Write(int8_t(0));
			writer.WriteAt(top.patchSizeAt, int32_t(writer.GetHave() - top.patchSizeAt));
			stack.pop();
			continue;
		}
		auto childIndex = top.childIndex;
		top.childIndex += 1;
		const Bson *child = nullptr;
		String elementName;
		if (value.Is<Object>())
		{
			auto it = top.childIterator;
			++top.childIterator;
			elementName = it->first;
			child = &it->second;
		}
		else
		{
			elementName = ByteString::Build(childIndex);
			child = value.Get(std::size_t(childIndex));
		}
		if (nonconformance && nonconformance->Is<Array>() && nonconformance->Get(0)->As<String>() == "arrayEncodedWithBadKeys")
		{
			elementName = nonconformance->Get(1)->As<String>();
		}
		auto childType = child->GetType();
		auto elementTypeAt = writer.GetHave();
		writer.Write(int8_t(0));
		for (auto ch : elementName)
		{
			writer.Write(ch);
		}
		writer.Write(char(0));
		int32_t elementType = 0;
		switch (childType)
		{
		default:
			{
				elementType = childType == Type::objectValue ? 3 : 4;
				auto *childNonconformance = (nonconformance && nonconformance->Is<Object>()) ? nonconformance->Get(elementName) : nullptr;
				if (childNonconformance && childNonconformance->Is<Array>() && childNonconformance->Get(0)->As<String>() == "objectEncodedAsArray")
				{
					elementType = 4;
				}
				auto sizeAt = writer.GetHave();
				writer.Write(int32_t(0));
				auto childIterator = child->Is<Object>() ? child->As<Object>().begin() : Object::const_iterator{};
				stack.push({ child, childNonconformance, sizeAt, 0, childIterator });
			}
			break;

		case Type::stringValue:
			{
				elementType = 2;
				auto &str = child->As<String>();
				if (str.size() > INT32_MAX - 1)
				{
					throw DumpError("string too big");
				}
				auto data = std::span(str.begin(), str.end());
				writer.Write(int32_t(data.size()) + 1);
				writer.data.insert(writer.data.end(), data.begin(), data.end());
				writer.Write(char(0));
			}
			break;

		case Type::userValue:
			{
				elementType = 5;
				auto &user = child->As<User>();
				if (user.size() > INT32_MAX)
				{
					throw DumpError("userdata too big");
				}
				writer.Write(int32_t(user.size()));
				writer.Write(uint8_t(128));
				auto have = writer.GetHave();
				writer.data.resize(writer.data.size() + user.size());
				std::copy(user.begin(), user.end(), writer.data.begin() + have);
			}
			break;

		case Type::doubleValue: elementType =  1; writer.Write(        child->As<Double>() ); break;
		case Type::int32Value : elementType = 16; writer.Write(int32_t(child->As<Int32> ())); break;
		case Type::int64Value : elementType = 18; writer.Write(int64_t(child->As<Int64> ())); break;
		case Type::boolValue  : elementType =  8; writer.Write(uint8_t(child->As<Bool>  ())); break;
		}
		writer.WriteAt(elementTypeAt, int8_t(elementType));
	}
	return writer.data;
}
