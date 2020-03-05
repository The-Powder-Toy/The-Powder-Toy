#include "ElementCommon.h"

std::vector<Element> const &GetElements()
{
	struct DoOnce
	{
		std::vector<Element> elements;

		DoOnce() : elements(PT_NUM)
		{
#define ELEMENT_NUMBERS_CALL
#include "ElementNumbers.h"
#undef ELEMENT_NUMBERS_CALL
		}
	};

	static DoOnce doOnce;
	return doOnce.elements;
}
