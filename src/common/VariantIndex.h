#pragma once
#include <variant>
#include <utility>
#include <stdexcept>

// https://stackoverflow.com/a/52303671
template<typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t VariantIndex()
{
	static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
	if constexpr (index == std::variant_size_v<VariantType>)
	{
		return index;
	}
	else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>)
	{
		return index;
	}
	else
	{
		return VariantIndex<VariantType, T, index + 1>();
	}
}

// https://stackoverflow.com/a/60564132
template<typename VariantType, std::size_t current = 0>
VariantType VariantFromIndex(std::size_t index)
{
	if constexpr (current >= std::variant_size_v<VariantType>)
	{
		throw std::runtime_error{"Variant index " + std::to_string(current + index) + " out of bounds"};
	}
	else if (index == 0)
	{
		return VariantType{ std::in_place_index<current> };
	}
	else
	{
		return VariantFromIndex<VariantType, current + 1>(index - 1);
	}
}

template<class Thing, class VariantType>
concept VariantAlternative = std::is_constructible_v<VariantType, Thing>;
