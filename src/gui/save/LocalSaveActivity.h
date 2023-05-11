#pragma once

#include <functional>
#include <memory>
#include <vector>
#include "Activity.h"
#include "client/SaveFile.h"
#include "common/Plane.h"
#include "Format.h"
#include "graphics/Pixel.h"

#include "save_local.png.h"

namespace ui
{
	class Textbox;
}

class VideoBuffer;

class ThumbnailRendererTask;

class LocalSaveActivity: public WindowActivity
{
	using OnSaved = std::function<void (std::unique_ptr<SaveFile>)>;
	std::unique_ptr<PlaneAdapter<std::vector<pixel_rgba>>> saveToDiskImage = format::PixelsFromPNG(
		std::vector<char>(save_local_png, save_local_png + save_local_png_size)
	);

	std::unique_ptr<SaveFile> save;
	ThumbnailRendererTask *thumbnailRenderer;
	std::unique_ptr<VideoBuffer> thumbnail;
	ui::Textbox * filenameField;
	OnSaved onSaved;

public:
	LocalSaveActivity(std::unique_ptr<SaveFile> newSave, OnSaved onSaved = nullptr);
	void saveWrite(ByteString finalFilename);
	void Save();
	void OnDraw() override;
	void OnTick(float dt) override;
	virtual ~LocalSaveActivity();
};
