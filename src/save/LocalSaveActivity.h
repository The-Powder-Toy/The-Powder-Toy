#pragma once

#include "Activity.h"
#include "client/SaveFile.h"
#include "client/ThumbnailListener.h"

namespace ui
{
	class Textbox;
}

class Thumbnail;

class LocalSaveActivity: public WindowActivity, public ThumbnailListener
{
	SaveFile save;
	Thumbnail * thumbnail;
	ui::Textbox * filenameField;
	class CancelAction;
	class SaveAction;
	friend class CancelAction;
	friend class SaveAction;
public:
	LocalSaveActivity(SaveFile save);
	void saveWrite(std::string finalFilename);
	virtual void Save();
	virtual void OnDraw();
	virtual void OnThumbnailReady(Thumbnail * thumbnail);
	virtual ~LocalSaveActivity();
};