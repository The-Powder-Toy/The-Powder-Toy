#pragma once

#include <vector>
#include <string>
#include "Activity.h"
#include "gui/interface/Window.h"
#include "tasks/TaskListener.h"


class SaveFile;
class FileSelectedCallback
{
public:
	FileSelectedCallback() {}
	virtual  ~FileSelectedCallback() {}
	virtual void FileSelected(SaveFile* file) {}
};

namespace ui
{
	class Label;
	class ScrollPanel;
	class ProgressBar;
}

class LoadFilesTask;
class FileBrowserActivity: public TaskListener, public WindowActivity
{
	LoadFilesTask * loadFiles;
	FileSelectedCallback * callback;
	ui::ScrollPanel * itemList;
	ui::Label * infoText;
	std::vector<SaveFile*> files;
	std::vector<ui::Component*> components;
	std::vector<ui::Component*> componentsQueue;
	std::string directory;

	ui::ProgressBar * progressBar;

	int totalFiles;
	int filesX, filesY, buttonPadding;
	int fileX, fileY;
	int buttonWidth, buttonHeight, buttonAreaWidth, buttonAreaHeight, buttonXOffset, buttonYOffset;


	class SearchAction;
	void populateList();
public:
	FileBrowserActivity(std::string directory, FileSelectedCallback * callback);
	virtual void OnDraw();
	virtual void OnTick(float dt);
	virtual void OnTryExit(ExitMethod method);
	virtual void OnMouseDown(int x, int y, unsigned button);
	void loadDirectory(std::string directory, std::string search);
	void SelectSave(SaveFile * file);
	void DeleteSave(SaveFile * file);
	void RenameSave(SaveFile * file);
	void DoSearch(std::string search);
	virtual ~FileBrowserActivity();

	virtual void NotifyDone(Task * task);
	virtual void NotifyError(Task * task);
	virtual void NotifyProgress(Task * task);
	virtual void NotifyStatus(Task * task);
};
