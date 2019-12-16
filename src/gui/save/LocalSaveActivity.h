#pragma once

#include "Activity.h"
#include "client/SaveFile.h"

#include <functional>

namespace ui
{
	class Textbox;
}

class VideoBuffer;

class ThumbnailRendererTask;

class LocalSaveActivity: public WindowActivity
{
	using OnSaved = std::function<void (SaveFile *)>;

	SaveFile save;
	ThumbnailRendererTask *thumbnailRenderer;
	std::unique_ptr<VideoBuffer> thumbnail;
	ui::Textbox * filenameField;
	OnSaved onSaved;
	
public:
	LocalSaveActivity(SaveFile save, OnSaved onSaved = nullptr);
	void saveWrite(ByteString finalFilename);
	void Save();
	void OnDraw() override;
	void OnTick(float dt) override;
	virtual ~LocalSaveActivity();
};
