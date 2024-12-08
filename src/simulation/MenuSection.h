#pragma once
#include "common/String.h"

struct menu_section
{
	String::value_type icon;
	String name;
	int itemcount;
	int doshow;
};

constexpr int SC_WALL      =  0;
constexpr int SC_ELEC      =  1;
constexpr int SC_POWERED   =  2;
constexpr int SC_SENSOR    =  3;
constexpr int SC_FORCE     =  4;
constexpr int SC_EXPLOSIVE =  5;
constexpr int SC_GAS       =  6;
constexpr int SC_LIQUID    =  7;
constexpr int SC_POWDERS   =  8;
constexpr int SC_SOLIDS    =  9;
constexpr int SC_NUCLEAR   = 10;
constexpr int SC_SPECIAL   = 11;
constexpr int SC_LIFE      = 12;
constexpr int SC_TOOL      = 13;
constexpr int SC_FAVORITES = 14;
constexpr int SC_DECO      = 15;
