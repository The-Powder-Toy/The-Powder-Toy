#pragma once
#include "DependentFalse.h"
#include <cstddef>
#include <limits>
#include <new>
#include <utility>

template<class ...Alternatives> struct OurVariantStorage;
template<> struct OurVariantStorage<>
{
	template<std::size_t> void DefaultInit(std::size_t) {}
	template<std::size_t> void Uninit(std::size_t) {}
	template<std::size_t Index, class Alternative> static constexpr bool CanGetStaticTag() { return false; }
	template<std::size_t Index, class Alternative> static constexpr std::size_t GetStaticTag() { static_assert(DependentFalse<Alternative>::value); return std::numeric_limits<std::size_t>::max(); }
	template<std::size_t> void CopyFrom(std::size_t, const OurVariantStorage &) {}
	template<std::size_t> void MoveFrom(std::size_t, OurVariantStorage &) noexcept {}
	template<std::size_t> bool Equal(std::size_t, const OurVariantStorage &) const { return false; }
};
template<class First, class ...Rest>
struct OurVariantStorage<First, Rest...>
{
	using RestStorage = OurVariantStorage<Rest...>;
	union { First first; RestStorage rest; };
	OurVariantStorage() {} // nothing, storage managed by OurVariant
	~OurVariantStorage() {} // nothing, storage managed by OurVariant
	template<class Alternative> const Alternative &Get() const { if constexpr (std::is_same_v<First, Alternative>) return first; else return rest.template Get<Alternative>(); }
	template<class Alternative>       Alternative &Get()       { if constexpr (std::is_same_v<First, Alternative>) return first; else return rest.template Get<Alternative>(); }
	template<std::size_t Index> void DefaultInit(std::size_t index) { if (Index == index) { new (&first) First(); return; } rest.template DefaultInit<Index + 1>(index); }
	template<std::size_t Index> void Uninit(std::size_t index) { if (Index == index) { first.~First(); return; } rest.template Uninit<Index + 1>(index); }
	template<std::size_t Index, class Alternative> static constexpr bool CanGetStaticTag() { if constexpr (std::is_same_v<First, Alternative>) return true; else return RestStorage::template CanGetStaticTag<Index + 1, Alternative>(); }
	template<std::size_t Index, class Alternative> static constexpr std::size_t GetStaticTag() { if constexpr (std::is_same_v<First, Alternative>) return Index; else return RestStorage::template GetStaticTag<Index + 1, Alternative>(); }
	template<std::size_t Index, std::size_t AlternativeIndex, class Alternative> void CopyFromAlternative(const Alternative &alternative) { if constexpr (Index == AlternativeIndex) new (&first) First(alternative); else rest.template CopyFromAlternative<Index + 1, AlternativeIndex>(alternative); }
	template<std::size_t Index, std::size_t AlternativeIndex, class Alternative> void MoveFromAlternative(Alternative &alternative) noexcept { if constexpr (Index == AlternativeIndex) new (&first) First(std::move(alternative)); else rest.template MoveFromAlternative<Index + 1, AlternativeIndex>(alternative); }
	template<std::size_t Index> void CopyFrom(std::size_t index, const OurVariantStorage &other) { if (Index == index) { new (&first) First(other.first); return; } rest.template CopyFrom<Index + 1>(index, other.rest); }
	template<std::size_t Index> void MoveFrom(std::size_t index, OurVariantStorage &other) noexcept { if (Index == index) { new (&first) First(std::move(other.first)); return; } rest.template MoveFrom<Index + 1>(index, other.rest); }
	template<std::size_t Index> bool Equal(std::size_t index, const OurVariantStorage &other) const { if (Index == index) return first == other.first; return rest.template Equal<Index + 1>(index, other.rest); }
};
template<class ...Alternatives>
class OurVariant
{
	using Storage = OurVariantStorage<Alternatives...>;
	Storage storage;
	std::size_t tag;

public:
	friend void swap(OurVariant &one, OurVariant &other) noexcept
	{
		auto tempTag = one.tag;
		Storage tempStorage;
		tempStorage.template MoveFrom<0>(tempTag, one.storage);
		one.tag = other.tag;
		one.storage.template MoveFrom<0>(one.tag, other.storage);
		other.tag = tempTag;
		other.storage.template MoveFrom<0>(other.tag, tempStorage);
	}

	struct Tag { std::size_t tag; };
	OurVariant(Tag fromTag = { 0 }) : tag(fromTag.tag) { storage.template DefaultInit<0>(tag); }
	OurVariant(const OurVariant &other) : tag(other.tag) { storage.template CopyFrom<0>(tag, other.storage); }
	~OurVariant() { storage.template Uninit<0>(tag); }
	OurVariant &operator=(OurVariant other) { swap(*this, other); return *this; }
	OurVariant(OurVariant &&other) noexcept : OurVariant() { swap(*this, other); }
	template<class Alternative> OurVariant(const Alternative  &alternative) { constexpr auto AlternativeIndex = Storage::template GetStaticTag<0, Alternative>(); tag = AlternativeIndex; storage.template CopyFromAlternative<0, AlternativeIndex>(alternative); }
	template<class Alternative> OurVariant(      Alternative &&alternative) { constexpr auto AlternativeIndex = Storage::template GetStaticTag<0, Alternative>(); tag = AlternativeIndex; storage.template MoveFromAlternative<0, AlternativeIndex>(alternative); }
	template<class Alternative> const Alternative &Get() const { return storage.template Get<Alternative>(); }
	template<class Alternative> Alternative &Get() { return storage.template Get<Alternative>(); }
	template<class Alternative> bool Holds() const { return tag == GetStaticTag<Alternative>(); }
	template<class Alternative> const Alternative *GetIf() const { if (!Holds<Alternative>()) return nullptr; return &storage.template Get<Alternative>(); }
	template<class Alternative>       Alternative *GetIf()       { if (!Holds<Alternative>()) return nullptr; return &storage.template Get<Alternative>(); }
	template<class Alternative> static constexpr bool CanGetStaticTag() { return Storage::template CanGetStaticTag<0, Alternative>(); }
	template<class Alternative> static constexpr std::size_t GetStaticTag() { return Storage::template GetStaticTag<0, Alternative>(); }
	bool operator ==(const OurVariant &other) const { if (tag != other.tag) return false; return storage.template Equal<0>(tag, other.storage); }
	std::size_t GetTag() const { return tag; }
};
template<class Thing, class VariantType>
concept OurVariantAlternative = VariantType::template CanGetStaticTag<Thing>();
