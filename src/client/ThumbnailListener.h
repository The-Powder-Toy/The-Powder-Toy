#pragma once

class Thumbnail;
class ThumbnailListener
{
public:
	ThumbnailListener() {}
	virtual ~ThumbnailListener() {}

	virtual void OnThumbnailReady(Thumbnail * thumb) {}
};
