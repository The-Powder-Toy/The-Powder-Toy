#pragma once
#include "common/String.h"

struct menu_section
{
	String::value_type icon;
	String name;
	int itemcount;
	int doshow;
};
