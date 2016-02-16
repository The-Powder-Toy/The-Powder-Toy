#ifndef WALLTYPE_H_
#define WALLTYPE_H_

#include "graphics/Graphics.h"
class VideoBuffer;

struct wall_type
{
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	VideoBuffer * (*textureGen)(int, int, int);
	const wchar_t *name;
	const char *identifier;
	const wchar_t *descs;
};

#endif
