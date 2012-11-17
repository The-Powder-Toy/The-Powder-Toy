#pragma once

class Thumbnail;
class ThumbnailListener
{
public:
	int ListenerRand;
	ThumbnailListener() { ListenerRand = rand(); }
	virtual ~ThumbnailListener() {}

	virtual void OnThumbnailReady(Thumbnail * thumb) {}
};
