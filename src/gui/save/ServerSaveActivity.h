#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Activity.h"
#include "client/SaveInfo.h"
#include "common/Plane.h"
#include "Format.h"
#include "graphics/Pixel.h"
#include "tasks/TaskListener.h"

#include "save_online.png.h"

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
	using OnUploaded = std::function<void (std::unique_ptr<SaveInfo>)>;
	std::unique_ptr<PlaneAdapter<std::vector<pixel_rgba>>> saveToServerImage = format::PixelsFromPNG(
		std::vector<char>(save_online_png, save_online_png + save_online_png_size)
	);

public:
	ServerSaveActivity(std::unique_ptr<SaveInfo> newSave, OnUploaded onUploaded);
	ServerSaveActivity(std::unique_ptr<SaveInfo> newSave, bool saveNow, OnUploaded onUploaded);
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
	std::unique_ptr<SaveInfo> save;
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
