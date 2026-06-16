#pragma once
#include <cstdint>
#include <map>
#include <span>
#include <stdexcept>
#include <vector>
#include "common/OurVariant.h"
#include "common/String.h"

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
	using Wrapped = OurVariant<
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
		objectValue = Wrapped::GetStaticTag<Object>(),
		arrayValue  = Wrapped::GetStaticTag<Array >(),
		stringValue = Wrapped::GetStaticTag<String>(),
		userValue   = Wrapped::GetStaticTag<User  >(),
		doubleValue = Wrapped::GetStaticTag<Double>(),
		int32Value  = Wrapped::GetStaticTag<Int32 >(),
		int64Value  = Wrapped::GetStaticTag<Int64 >(),
		boolValue   = Wrapped::GetStaticTag<Bool  >(),
	}; // keep this in sync with Wrapped's definition

	Bson()                              = default;
	Bson(const Bson &other)             = default;
	Bson(Bson &&other)                  = default;
	Bson &operator =(const Bson &other) = default;
	Bson &operator =(Bson &&other)      = default;

	Bson(Type type) : wrapped(Wrapped::Tag{ std::size_t(type) }) {}

	Bson(const char *ch) : Bson(String(ch)) {}

	template<OurVariantAlternative<Wrapped> Alternative> Bson(      Alternative &&newWrapped) : wrapped(std::forward<Alternative>(newWrapped)) {}
	template<OurVariantAlternative<Wrapped> Alternative> Bson(const Alternative  &newWrapped) : wrapped(                          newWrapped ) {}

	template<OurVariantAlternative<Wrapped> Alternative> void Ensure() { if (!wrapped.Holds<Alternative>()) wrapped = Alternative{}; }

	Bson &operator [](std::size_t   key) { Ensure<Array >(); return wrapped.Get<Array >()[key]; }
	Bson &operator [](const String &key) { Ensure<Object>(); return wrapped.Get<Object>()[key]; }

	const Bson *Get(std::size_t   key) const { auto &arr = wrapped.Get<Array >();                          if (key >= arr.size()) return nullptr; return &arr[key]  ; }
	const Bson *Get(const String &key) const { auto &obj = wrapped.Get<Object>(); auto it = obj.find(key); if (it  == obj.end() ) return nullptr; return &it->second; }

	template<OurVariantAlternative<Wrapped> Alternative> const Alternative Get(std::size_t   key, Alternative def) const { if (auto *node = Get(key)) { return node->As<Alternative>(); } return def; }
	template<OurVariantAlternative<Wrapped> Alternative> const Alternative Get(const String &key, Alternative def) const { if (auto *node = Get(key)) { return node->As<Alternative>(); } return def; }

	const Bson Get(std::size_t   key, Bson def) const { if (auto *node = Get(key)) { return *node; } return def; }
	const Bson Get(const String &key, Bson def) const { if (auto *node = Get(key)) { return *node; } return def; }

	template<OurVariantAlternative<Wrapped> Alternative>       Alternative &As()       { return wrapped.Get<Alternative>(); }
	template<OurVariantAlternative<Wrapped> Alternative> const Alternative &As() const { return wrapped.Get<Alternative>(); }

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

	Type GetType() const { return Type(wrapped.GetTag()); }

	template<class Alternative> bool Is() const { return wrapped.Holds<Alternative>(); }

	std::size_t GetSize() const
	{
		if (auto *object = wrapped.GetIf<Object>()) return object->size();
		return wrapped.Get<Array>().size();
	}

	bool IsEmpty() const
	{
		if (auto *object = wrapped.GetIf<Object>()) return object->empty();
		return wrapped.Get<Array>().empty();
	}

	Bson &Append(const Bson  &value) { Ensure<Array>(); return wrapped.Get<Array>().emplace_back(                   value ); };
	Bson &Append(      Bson &&value) { Ensure<Array>(); return wrapped.Get<Array>().emplace_back(std::forward<Bson>(value)); };

	bool operator ==(const Bson &other) const
	{
		return wrapped == other.wrapped;
	}
};
