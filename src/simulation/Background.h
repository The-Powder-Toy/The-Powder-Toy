#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#include "common/String.h"
#include "graphics/Pixel.h"
class VideoBuffer;

struct background
{
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	VideoBuffer * (*textureGen)(int, int, int);
	String name;
	ByteString identifier;
	String descs;
};

#endif
