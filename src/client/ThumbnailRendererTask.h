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
	std::unique_ptr<VideoBuffer> thumbnail;

	static int queueSize;

public:
	ThumbnailRendererTask(GameSave const &, Vec2<int> size, bool decorations, bool fire);
	virtual ~ThumbnailRendererTask();

	virtual bool doWork() override;
	std::unique_ptr<VideoBuffer> Finish();

	static int QueueSize();
};
