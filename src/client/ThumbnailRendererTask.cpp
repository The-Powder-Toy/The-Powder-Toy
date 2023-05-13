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

ThumbnailRendererTask::ThumbnailRendererTask(GameSave const &save, Vec2<int> size, bool decorations, bool fire):
	save(std::make_unique<GameSave>(save)),
	size(size),
	decorations(decorations),
	fire(fire)
{
	queueSize += 1;
}

ThumbnailRendererTask::~ThumbnailRendererTask()
{
	queueSize -= 1;
}

bool ThumbnailRendererTask::doWork()
{
	std::tie(thumbnail, std::ignore) = SaveRenderer::Ref().Render(save.get(), decorations, fire);
	if (thumbnail)
	{
		thumbnail->ResizeToFit(size, true);
		size = thumbnail->Size();
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

