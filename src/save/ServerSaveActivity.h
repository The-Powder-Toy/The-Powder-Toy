#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "client/ThumbnailListener.h"
#include "tasks/TaskListener.h"

namespace ui
{
	class Textbox;
	class Checkbox;
}

class Task;
class Thumbnail;
class ServerSaveActivity: public WindowActivity, public ThumbnailListener, public TaskListener
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
	ServerSaveActivity(SaveInfo save, bool saveNow, SaveUploadedCallback * callback);
	void saveUpload();
	virtual void Save();
	virtual void Exit();
	virtual void OnDraw();
	virtual void OnThumbnailReady(Thumbnail * thumbnail);
	virtual void OnTick(float dt);
	virtual ~ServerSaveActivity();
protected:
	virtual void NotifyDone(Task * task);
	Task * saveUploadTask;
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