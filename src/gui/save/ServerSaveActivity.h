#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "client/requestbroker/RequestListener.h"
#include "tasks/TaskListener.h"

namespace ui
{
	class Label;
	class Textbox;
	class Checkbox;
}

class Task;
class Thumbnail;
class VideoBuffer;
class ServerSaveActivity: public WindowActivity, public RequestListener, public TaskListener
{
public:
	class SaveUploadedCallback
	{
	public:
		SaveUploadedCallback() = default;
		virtual  ~SaveUploadedCallback() = default;
		virtual void SaveUploaded(SaveInfo save) {}
	};
	ServerSaveActivity(SaveInfo save, SaveUploadedCallback * callback);
	ServerSaveActivity(SaveInfo save, bool saveNow, SaveUploadedCallback * callback);
	void saveUpload();
	virtual void Save();
	void Exit() override;
	virtual void ShowPublishingInfo();
	virtual void ShowRules();
	virtual void CheckName(std::string newname);
	void OnDraw() override;
	void OnResponseReady(void * imagePtr, int identifier) override;
	void OnTick(float dt) override;
	~ServerSaveActivity() override;
protected:
	void AddAuthorInfo();
	void NotifyDone(Task * task) override;
	VideoBuffer * thumbnail;
	SaveInfo save;
	SaveUploadedCallback * callback;
	Task * saveUploadTask;
	ui::Label * titleLabel;
	ui::Textbox * nameField;
	ui::Textbox * descriptionField;
	ui::Checkbox * publishedCheckbox;
	ui::Checkbox * pausedCheckbox;
	class CancelAction;
	class SaveAction;
	class PublishingAction;
	class RulesAction;
	class NameChangedAction;
};
