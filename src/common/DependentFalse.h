#pragma once
#include <type_traits>

template<class>
struct DependentFalse : std::false_type
{
};
