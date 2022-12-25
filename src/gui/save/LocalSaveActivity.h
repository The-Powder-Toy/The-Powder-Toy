#pragma once

#include "Activity.h"
#include "client/SaveFile.h"
#include "graphics/Pixel.h"

#include <functional>
#include <memory>
#include <vector>

namespace ui
{
	class Textbox;
}

class VideoBuffer;

class ThumbnailRendererTask;

class LocalSaveActivity: public WindowActivity
{
	using OnSaved = std::function<void (SaveFile *)>;
	std::vector<pixel> save_to_disk_image;
	int save_to_disk_imageW, save_to_disk_imageH;

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
