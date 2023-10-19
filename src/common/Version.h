#pragma once
#include <array>
#include <algorithm>

template<size_t ComponentCount>
struct Version
{
	std::array<size_t, ComponentCount> components;

	template<class ...Args>
	constexpr Version(Args ...args) : components{ size_t(args)... }
	{
	}

	constexpr bool operator <(const Version &other) const
	{
		return std::lexicographical_compare(components.begin(), components.end(), other.components.begin(), other.components.end());
	}

	constexpr bool operator ==(const Version &other) const
	{
		return std::equal(components.begin(), components.end(), other.components.begin(), other.components.end());
	}

	constexpr bool operator <=(const Version &other) const
	{
		return *this < other || *this == other;
	}

	constexpr size_t operator [](size_t index) const
	{
		return components[index];
	}

	size_t &operator [](size_t index)
	{
		return components[index];
	}
};

template<class ...Args>
Version(Args ...args) -> Version<sizeof...(Args)>;
