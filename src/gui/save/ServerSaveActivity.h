#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "client/requestbroker/RequestListener.h"
#include "tasks/TaskListener.h"

namespace ui
{
	class Textbox;
	class Checkbox;
}

class Task;
class Thumbnail;
class ServerSaveActivity: public WindowActivity, public RequestListener, public TaskListener
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
	virtual void ShowPublishingInfo();
	virtual void ShowRules();
	virtual void OnDraw();
	virtual void OnResponseReady(void * imagePtr, int identifier);
	virtual void OnTick(float dt);
	virtual ~ServerSaveActivity();
protected:
	virtual void NotifyDone(Task * task);
	Task * saveUploadTask;
	SaveUploadedCallback * callback;
	SaveInfo save;
	VideoBuffer * thumbnail;
	ui::Textbox * nameField;
	ui::Textbox * descriptionField;
	ui::Checkbox * publishedCheckbox;
	ui::Checkbox * pausedCheckbox;
	class CancelAction;
	class SaveAction;
	class PublishingAction;
	class RulesAction;
};
