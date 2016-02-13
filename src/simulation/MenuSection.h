#ifndef MENUSECTION_H_
#define MENUSECTION_H_

struct menu_section
{
	const char *icon;
	const wchar_t *name;
	int itemcount;
	int doshow;
};

#endif
