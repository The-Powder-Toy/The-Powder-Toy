#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <stdexcept>
#include <variant>
#include <vector>
#include "common/String.h"
#include "common/VariantIndex.h"

class Bson
{
public:
	using String = ByteString;
	using Double = double;
	using Int32  = int32_t;
	using Int64  = int64_t;
	using Bool   = bool;
	using User   = std::vector<unsigned char>;

	using Object = std::map<String, Bson>;
	using Array  = std::vector<Bson>;

private:
	using Wrapped = std::variant<
		Object,
		Array,
		String,
		User,
		Double,
		Int32,
		Int64,
		Bool
	>;
	Wrapped wrapped;

public:
	enum class Type
	{
		objectValue = VariantIndex<Wrapped, Object>(),
		arrayValue  = VariantIndex<Wrapped, Array >(),
		stringValue = VariantIndex<Wrapped, String>(),
		userValue   = VariantIndex<Wrapped, User  >(),
		doubleValue = VariantIndex<Wrapped, Double>(),
		int32Value  = VariantIndex<Wrapped, Int32 >(),
		int64Value  = VariantIndex<Wrapped, Int64 >(),
		boolValue   = VariantIndex<Wrapped, Bool  >(),
	}; // keep this in sync with Wrapped's definition

	Bson()                              = default;
	Bson(const Bson &other)             = default;
	Bson(Bson &&other)                  = default;
	Bson &operator =(const Bson &other) = default;
	Bson &operator =(Bson &&other)      = default;

	Bson(Type type) : wrapped(VariantFromIndex<Wrapped>(std::size_t(type))) {}

	template<VariantAlternative<Wrapped> Alternative> Bson(      Alternative &&newWrapped) : wrapped(std::forward<Alternative>(newWrapped)) {}
	template<VariantAlternative<Wrapped> Alternative> Bson(const Alternative  &newWrapped) : wrapped(                          newWrapped ) {}

	template<VariantAlternative<Wrapped> Alternative> void Ensure() { if (!std::holds_alternative<Alternative>(wrapped)) wrapped = Alternative{}; }

	Bson &operator [](std::size_t   key) { Ensure<Array >(); return std::get<Array >(wrapped)[key]; }
	Bson &operator [](const String &key) { Ensure<Object>(); return std::get<Object>(wrapped)[key]; }

	const Bson *Get(std::size_t   key) const { auto &arr = std::get<Array >(wrapped);                          if (key >= arr.size()) return nullptr; return &arr[key]  ; }
	const Bson *Get(const String &key) const { auto &obj = std::get<Object>(wrapped); auto it = obj.find(key); if (it  == obj.end() ) return nullptr; return &it->second; }

	template<VariantAlternative<Wrapped> Alternative> const Alternative Get(std::size_t   key, Alternative def) const { if (auto *node = Get(key)) { return node->As<Alternative>(); } return def; }
	template<VariantAlternative<Wrapped> Alternative> const Alternative Get(const String &key, Alternative def) const { if (auto *node = Get(key)) { return node->As<Alternative>(); } return def; }

	const Bson Get(std::size_t   key, Bson def) const { if (auto *node = Get(key)) { return *node; } return def; }
	const Bson Get(const String &key, Bson def) const { if (auto *node = Get(key)) { return *node; } return def; }

	template<VariantAlternative<Wrapped> Alternative>       Alternative &As()       { return std::get<Alternative>(wrapped); }
	template<VariantAlternative<Wrapped> Alternative> const Alternative &As() const { return std::get<Alternative>(wrapped); }

	struct ParseError : std::runtime_error
	{
		using runtime_error::runtime_error;
	};
	static Bson Parse(std::span<const char> data, const Bson *rootNonconformance = nullptr);

	struct DumpError : std::runtime_error
	{
		using runtime_error::runtime_error;
	};
	std::vector<char> Dump(const Bson *rootNonconformance = nullptr) const;

	Type GetType() const { return Type(wrapped.index()); }

	template<class Alternative> bool Is() const { return std::holds_alternative<Alternative>(wrapped); }

	std::size_t GetSize() const
	{
		if (auto *object = std::get_if<Object>(&wrapped)) return object->size();
		return std::get<Array>(wrapped).size();
	}

	bool IsEmpty() const
	{
		if (auto *object = std::get_if<Object>(&wrapped)) return object->empty();
		return std::get<Array>(wrapped).empty();
	}

	Bson &Append(const Bson  &value) { Ensure<Array>(); return std::get<Array>(wrapped).emplace_back(                   value ); };
	Bson &Append(      Bson &&value) { Ensure<Array>(); return std::get<Array>(wrapped).emplace_back(std::forward<Bson>(value)); };

	bool operator ==(const Bson &other) const = default;
};
