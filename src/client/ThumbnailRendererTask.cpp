#include "ThumbnailRendererTask.h"

#include <cmath>

#include "graphics/Graphics.h"
#include "simulation/SaveRenderer.h"
#include "client/GameSave.h"

int ThumbnailRendererTask::queueSize = 0;

int ThumbnailRendererTask::QueueSize()
{
	return queueSize;
}

ThumbnailRendererTask::ThumbnailRendererTask(GameSave const &save, Vec2<int> size, bool autoRescale, bool decorations, bool fire):
	save(std::make_unique<GameSave>(save)),
	size(size),
	decorations(decorations),
	fire(fire),
	autoRescale(autoRescale)
{
	queueSize += 1;
}

ThumbnailRendererTask::ThumbnailRendererTask(GameSave *save, int width, int height, bool autoRescale, bool decorations, bool fire):
	ThumbnailRendererTask(*save, Vec2(width, height), autoRescale, decorations, fire)
{}

ThumbnailRendererTask::~ThumbnailRendererTask()
{
	queueSize -= 1;
}

bool ThumbnailRendererTask::doWork()
{
	thumbnail = std::unique_ptr<VideoBuffer>(SaveRenderer::Ref().Render(save.get(), decorations, fire));
	if (thumbnail)
	{
		if (autoRescale)
		{
			thumbnail->ResizeToFit(size, true);
			size = thumbnail->Size();
		}
		else
		{
			thumbnail->Resize(size);
		}
		return true;
	}
	else
	{
		return false;
	}
}

std::unique_ptr<VideoBuffer> ThumbnailRendererTask::Finish()
{
	auto ptr = std::move(thumbnail);
	AbandonableTask::Finish();
	return ptr;
}

