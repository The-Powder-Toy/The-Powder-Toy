#pragma once
#include "common/Vec2.h"
#include "tasks/AbandonableTask.h"

#include <memory>

class GameSave;
class VideoBuffer;
class ThumbnailRendererTask : public AbandonableTask
{
	std::unique_ptr<GameSave> save;
	Vec2<int> size;
	bool decorations;
	bool fire;
	bool autoRescale;
	std::unique_ptr<VideoBuffer> thumbnail;

	static int queueSize;

public:
	ThumbnailRendererTask(GameSave const &, Vec2<int> size, bool autoRescale, bool decorations, bool fire);
	[[deprecated("Use ThumbnailRendererTask(GameSave const &, Vec2<int>, bool, bool, bool)")]]
	ThumbnailRendererTask(GameSave *save, int width, int height, bool autoRescale = false, bool decorations = true, bool fire = true);
	virtual ~ThumbnailRendererTask();

	virtual bool doWork() override;
	std::unique_ptr<VideoBuffer> Finish();

	static int QueueSize();
};
