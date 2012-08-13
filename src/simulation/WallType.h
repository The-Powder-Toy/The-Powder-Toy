//
//  WallType.h
//  The Powder Toy
//
//  Created by Simon Robertshaw on 04/06/2012.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef The_Powder_Toy_WallType_h
#define The_Powder_Toy_WallType_h

#include "graphics/Graphics.h"
class VideoBuffer;

struct wall_type
{
	pixel colour;
	pixel eglow; // if emap set, add this to fire glow
	int drawstyle;
	VideoBuffer * (*textureGen)(int, int, int);
	const char *descs;
};

#endif
