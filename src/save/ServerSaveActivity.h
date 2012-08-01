#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "client/ThumbnailListener.h"

namespace ui
{
	class Textbox;
	class Checkbox;
}

class Thumbnail;
class ServerSaveActivity: public WindowActivity, public ThumbnailListener
{
public:
	class SaveUploadedCallback
	{
	public:
		SaveUploadedCallback() {}
		virtual  ~SaveUploadedCallback() {}
		virtual void SaveUploaded(SaveInfo save) {}
	};
	ServerSaveActivity(SaveInfo save, SaveUploadedCallback * callback);
	void saveUpload();
	virtual void Save();
	virtual void Exit();
	virtual void OnDraw();
	virtual void OnThumbnailReady(Thumbnail * thumbnail);
	virtual ~ServerSaveActivity();
protected:
	SaveUploadedCallback * callback;
	SaveInfo save;
	Thumbnail * thumbnail;
	ui::Textbox * nameField;
	ui::Textbox * descriptionField;
	ui::Checkbox * publishedCheckbox;
	class CancelAction;
	class SaveAction;
	friend class CancelAction;
	friend class SaveAction;
};