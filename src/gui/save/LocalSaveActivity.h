#pragma once

#include "Activity.h"
#include "client/SaveFile.h"
#include "client/requestbroker/RequestListener.h"

namespace ui
{
	class Textbox;
}

class VideoBuffer;

class FileSavedCallback
{
public:
	FileSavedCallback() {}
	virtual  ~FileSavedCallback() {}
	virtual void FileSaved(SaveFile * file) {}
};

class LocalSaveActivity: public WindowActivity, public RequestListener
{
	SaveFile save;
	VideoBuffer * thumbnail;
	ui::Textbox * filenameField;
	class CancelAction;
	class SaveAction;
	friend class CancelAction;
	friend class SaveAction;
	FileSavedCallback * callback;
public:
	LocalSaveActivity(SaveFile save, FileSavedCallback * callback);
	void saveWrite(std::string finalFilename);
	virtual void Save();
	virtual void OnDraw();
	virtual void OnResponseReady(void * imagePtr, int identifier);
	virtual ~LocalSaveActivity();
};
