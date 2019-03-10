#pragma once

#include "Activity.h"
#include "client/SaveFile.h"

#include <memory>

namespace ui
{
	class Textbox;
}

class VideoBuffer;

class ThumbnailRendererTask;
class FileSavedCallback
{
public:
	FileSavedCallback() {}
	virtual  ~FileSavedCallback() {}
	virtual void FileSaved(SaveFile * file) {}
};

class LocalSaveActivity: public WindowActivity
{
	SaveFile save;
	ThumbnailRendererTask *thumbnailRenderer;
	std::unique_ptr<VideoBuffer> thumbnail;
	ui::Textbox * filenameField;
	class CancelAction;
	class SaveAction;
	friend class CancelAction;
	friend class SaveAction;
	FileSavedCallback * callback;
public:
	LocalSaveActivity(SaveFile save, FileSavedCallback * callback);
	void saveWrite(ByteString finalFilename);
	void Save();
	void OnDraw() override;
	void OnTick(float dt) override;
	virtual ~LocalSaveActivity();
};
