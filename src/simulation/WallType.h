#ifndef WALLTYPE_H_
#define WALLTYPE_H_

#include "graphics/Pixel.h"
class VideoBuffer;

struct wall_type
{
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	VideoBuffer * (*textureGen)(int, int, int);
	const char *name;
	const char *identifier;
	const char *descs;
};

#endif
