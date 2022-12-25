#pragma once

#include "Activity.h"
#include "client/SaveInfo.h"
#include "tasks/TaskListener.h"
#include "graphics/Pixel.h"

#include <memory>
#include <functional>
#include <vector>

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
	using OnUploaded = std::function<void (SaveInfo &)>;
	std::vector<pixel> save_to_server_image;
	int save_to_server_imageW, save_to_server_imageH;

public:
	ServerSaveActivity(SaveInfo save, OnUploaded onUploaded);
	ServerSaveActivity(SaveInfo save, bool saveNow, OnUploaded onUploaded);
	void saveUpload();
	void Save();
	virtual void Exit() override;
	void ShowPublishingInfo();
	void ShowRules();
	void CheckName(String newname);
	virtual void OnDraw() override;
	virtual void OnTick(float dt) override;
	virtual ~ServerSaveActivity();
protected:
	void AddAuthorInfo();
	void NotifyDone(Task * task) override;
	ThumbnailRendererTask *thumbnailRenderer;
	std::unique_ptr<VideoBuffer> thumbnail;
	SaveInfo save;
private:
	OnUploaded onUploaded;
protected:
	Task * saveUploadTask;
	ui::Label * titleLabel;
	ui::Textbox * nameField;
	ui::Textbox * descriptionField;
	ui::Checkbox * publishedCheckbox;
	ui::Checkbox * pausedCheckbox;
};
