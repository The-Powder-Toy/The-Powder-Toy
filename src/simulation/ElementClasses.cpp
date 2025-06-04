#include "ElementCommon.h"
#include "ElementNumbers.h"

std::array<Element, PT_NUM> const &GetElements()
{
	struct DoOnce
	{
		std::array<Element, PT_NUM> elements;

		DoOnce()
		{
#define ELEMENT_NUMBERS_CALL(name, id) elements[id].Element_ ## name ();
			ELEMENT_NUMBERS(ELEMENT_NUMBERS_CALL)
#undef ELEMENT_NUMBERS_CALL
		}
	};

	static DoOnce doOnce;
	return doOnce.elements;
}
