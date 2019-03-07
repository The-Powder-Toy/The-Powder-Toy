#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "tasks/TaskListener.h"

#include <memory>

namespace ui
{
	class Label;
	class Textbox;
	class Checkbox;
}

class ThumbnailRendererTask;
class Task;
class VideoBuffer;
class ServerSaveActivity: public WindowActivity, public TaskListener
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
	virtual void CheckName(String newname);
	virtual void OnDraw();
	virtual void OnTick(float dt);
	virtual ~ServerSaveActivity();
protected:
	void AddAuthorInfo();
	virtual void NotifyDone(Task * task);
	std::unique_ptr<ThumbnailRendererTask> thumbnailRenderer;
	std::unique_ptr<VideoBuffer> thumbnail;
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
