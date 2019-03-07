#ifndef THUMBNAILRENDERER_H
#define THUMBNAILRENDERER_H

#include "tasks/Task.h"

#include <memory>

class GameSave;
class VideoBuffer;
class ThumbnailRendererTask : public Task
{
	std::unique_ptr<GameSave> Save;
	int Width, Height;
	bool Decorations;
	bool Fire;
	bool AutoRescale;
	std::unique_ptr<VideoBuffer> thumbnail;

public:
	ThumbnailRendererTask(GameSave *save, int width, int height, bool autoRescale = false, bool decorations = true, bool fire = true);
	virtual ~ThumbnailRendererTask();

	virtual bool doWork() override;
	std::unique_ptr<VideoBuffer> GetThumbnail();
};

#endif // THUMBNAILRENDERER_H

