#pragma once
#include <variant>
#include <utility>

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
