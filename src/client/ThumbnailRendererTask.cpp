#include "ThumbnailRendererTask.h"

#include <cmath>

#include "graphics/Graphics.h"
#include "simulation/SaveRenderer.h"
#include "client/GameSave.h"

ThumbnailRendererTask::ThumbnailRendererTask(GameSave *save, int width, int height, bool autoRescale, bool decorations, bool fire) :
	Save(new GameSave(*save)),
	Width(width),
	Height(height),
	Decorations(decorations),
	Fire(fire),
	AutoRescale(autoRescale)
{
}

ThumbnailRendererTask::~ThumbnailRendererTask()
{
}

bool ThumbnailRendererTask::doWork()
{
	thumbnail = std::unique_ptr<VideoBuffer>(SaveRenderer::Ref().Render(Save.get(), Decorations, Fire));
	if (thumbnail)
	{
		if (AutoRescale)
		{
			int scaleX = (int)std::ceil((float)thumbnail->Width / Width);
			int scaleY = (int)std::ceil((float)thumbnail->Height / Height);
			int scale = scaleX > scaleY ? scaleX : scaleY;
			int newWidth = thumbnail->Width / scale, newHeight = thumbnail->Height / scale;
			thumbnail->Resize(newWidth, newHeight, true);
			Width = newWidth;
			Height = newHeight;
		}
		else
		{
			thumbnail->Resize(Width, Height, true);
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

