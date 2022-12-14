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

ThumbnailRendererTask::ThumbnailRendererTask(GameSave *save, int width, int height, bool autoRescale, bool decorations, bool fire) :
	Save(new GameSave(*save)),
	Width(width),
	Height(height),
	Decorations(decorations),
	Fire(fire),
	AutoRescale(autoRescale)
{
	queueSize += 1;
}

ThumbnailRendererTask::~ThumbnailRendererTask()
{
	queueSize -= 1;
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
			newWidth = thumbnail->Width;
			newHeight = thumbnail->Height;
			if (newWidth > Width || newHeight > Height)
			{
				auto croppedWidth = newWidth > Width ? Width : newWidth;
				auto croppedHeight = newHeight > Height ? Height : newHeight;
				thumbnail->Crop(croppedWidth, croppedHeight, (newWidth - croppedWidth) / 2, (newHeight - croppedHeight) / 2);
				newWidth = thumbnail->Width;
				newHeight = thumbnail->Height;
			}
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

