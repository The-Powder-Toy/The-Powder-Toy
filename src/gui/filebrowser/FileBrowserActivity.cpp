#include "FileBrowserActivity.h"

#include <algorithm>

#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/ProgressBar.h"

#include "client/Client.h"
#include "client/SaveFile.h"
#include "client/GameSave.h"

#include "gui/Style.h"
#include "tasks/Task.h"

#include "gui/dialogues/TextPrompt.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"

#include "graphics/Graphics.h"

//Currently, reading is done on another thread, we can't render outside the main thread due to some bullshit with OpenGL
class LoadFilesTask: public Task
{
	ByteString directory;
	ByteString search;
	std::vector<SaveFile*> saveFiles;

	void before() override
	{

	}

	void after() override
	{

	}

	bool doWork() override
	{
		std::vector<ByteString> files = Client::Ref().DirectorySearch(directory, search, ".cps");
		std::sort(files.rbegin(), files.rend(), [](ByteString a, ByteString b) { return a.ToLower() < b.ToLower(); });

		notifyProgress(-1);
		for(std::vector<ByteString>::iterator iter = files.begin(), end = files.end(); iter != end; ++iter)
		{
			SaveFile * saveFile = new SaveFile(*iter);
			try
			{
				std::vector<unsigned char> data = Client::Ref().ReadFile(*iter);
				GameSave * tempSave = new GameSave(data);
				saveFile->SetGameSave(tempSave);
				saveFiles.push_back(saveFile);

				ByteString filename = (*iter).SplitFromEndBy(PATH_SEP).After();
				filename = filename.SplitFromEndBy('.').Before();
				saveFile->SetDisplayName(filename.FromUtf8());
			}
			catch(std::exception & e)
			{
				//:(
			}
		}
		return true;
	}

public:
	std::vector<SaveFile*> GetSaveFiles()
	{
		return saveFiles;
	}

	LoadFilesTask(ByteString directory, ByteString search):
		directory(directory),
		search(search)
	{

	}
};

FileBrowserActivity::FileBrowserActivity(ByteString directory, OnSelected onSelected_):
	WindowActivity(ui::Point(-1, -1), ui::Point(500, 350)),
	onSelected(onSelected_),
	directory(directory),
	totalFiles(0)
{

	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 18), "Save Browser");
	titleLabel->SetTextColour(style::Colour::WarningTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	ui::Textbox * textField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 16), "", "[search]");
	textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	textField->SetActionCallback({ [this, textField] { DoSearch(textField->GetText().ToUtf8()); } });
	AddComponent(textField);
	FocusComponent(textField);

	itemList = new ui::ScrollPanel(ui::Point(4, 45), ui::Point(Size.X-8, Size.Y-53));
	itemList->Visible = false;
	AddComponent(itemList);

	progressBar = new ui::ProgressBar(ui::Point((Size.X-200)/2, 45+(Size.Y-66)/2), ui::Point(200, 17));
	AddComponent(progressBar);

	infoText = new ui::Label(ui::Point((Size.X-200)/2, 45+(Size.Y-66)/2), ui::Point(200, 17), "No saves found");
	AddComponent(infoText);

	filesX = 4;
	filesY = 3;
	buttonPadding = 2;
	fileX = 0;
	fileY = 0;

	buttonXOffset = 0;
	buttonYOffset = 0;
	buttonAreaWidth = itemList->Size.X - 5;
	buttonAreaHeight = itemList->Size.Y;// - buttonYOffset - 18;
	buttonWidth = (buttonAreaWidth/filesX) - buttonPadding*2;
	buttonHeight = (buttonAreaHeight/filesY) - buttonPadding*2;

	loadDirectory(directory, "");
}

void FileBrowserActivity::DoSearch(ByteString search)
{
	if(!loadFiles)
	{
		loadDirectory(directory, search);
	}
}

void FileBrowserActivity::SelectSave(SaveFile * file)
{
	if (onSelected)
		onSelected(std::unique_ptr<SaveFile>(new SaveFile(*file)));
	Exit();
}

void FileBrowserActivity::DeleteSave(SaveFile * file)
{
	String deleteMessage = "Are you sure you want to delete " + file->GetDisplayName() + ".cps?";
	if (ConfirmPrompt::Blocking("Delete Save", deleteMessage))
	{
		remove(file->GetName().c_str());
		loadDirectory(directory, "");
	}
}

void FileBrowserActivity::RenameSave(SaveFile * file)
{
	ByteString newName = TextPrompt::Blocking("Rename", "Change save name", file->GetDisplayName(), "", 0).ToUtf8();
	if (newName.length())
	{
		newName = directory + PATH_SEP + newName + ".cps";
		int ret = rename(file->GetName().c_str(), newName.c_str());
		if (ret)
			ErrorMessage::Blocking("Error", "Could not rename file");
		else
			loadDirectory(directory, "");
	}
	else
		ErrorMessage::Blocking("Error", "No save name given");
}

void FileBrowserActivity::cleanup()
{
	for (auto comp : componentsQueue)
	{
		delete comp;
	}
	componentsQueue.clear();

	for (auto file : files)
	{
		delete file;
	}
	files.clear();
}

void FileBrowserActivity::loadDirectory(ByteString directory, ByteString search)
{
	for (size_t i = 0; i < components.size(); i++)
	{
		RemoveComponent(components[i]);
		itemList->RemoveChild(components[i]);
	}

	cleanup();

	infoText->Visible = false;
	itemList->Visible = false;
	progressBar->Visible = true;
	progressBar->SetProgress(-1);
	progressBar->SetStatus("Loading files");
	loadFiles = new LoadFilesTask(directory, search);
	loadFiles->AddTaskListener(this);
	loadFiles->Start();
}

void FileBrowserActivity::NotifyDone(Task * task)
{
	fileX = 0;
	fileY = 0;
	files = ((LoadFilesTask*)task)->GetSaveFiles();
	totalFiles = files.size();
	delete loadFiles;
	loadFiles = NULL;
	if (!files.size())
	{
		progressBar->Visible = false;
		infoText->Visible = true;
	}
	else
		itemList->Visible = true;
	for (size_t i = 0; i < components.size(); i++)
	{
		delete components[i];
	}
	components.clear();
}

void FileBrowserActivity::OnMouseDown(int x, int y, unsigned button)
{
	if (!(x > Position.X && y > Position.Y && y < Position.Y+Size.Y && x < Position.X+Size.X)) //Clicked outside window
		Exit();
}

void FileBrowserActivity::OnTryExit(ExitMethod method)
{
	Exit();
}

void FileBrowserActivity::NotifyError(Task * task)
{

}

void FileBrowserActivity::NotifyProgress(Task * task)
{
	progressBar->SetProgress(task->GetProgress());
}

void FileBrowserActivity::NotifyStatus(Task * task)
{

}

void FileBrowserActivity::OnTick(float dt)
{
	if(loadFiles)
		loadFiles->Poll();

	if(files.size())
	{
		SaveFile * saveFile = files.back();
		files.pop_back();

		if(fileX == filesX)
		{
			fileX = 0;
			fileY++;
		}
		ui::SaveButton * saveButton = new ui::SaveButton(
						ui::Point(
							buttonXOffset + buttonPadding + fileX*(buttonWidth+buttonPadding*2),
							buttonYOffset + buttonPadding + fileY*(buttonHeight+buttonPadding*2)
							),
						ui::Point(buttonWidth, buttonHeight),
						saveFile);
		saveButton->AddContextMenu(1);
		saveButton->Tick(dt);
		saveButton->SetActionCallback({
			[this, saveButton] { SelectSave(saveButton->GetSaveFile()); },
			[this, saveButton] { RenameSave(saveButton->GetSaveFile()); },
			[this, saveButton] { DeleteSave(saveButton->GetSaveFile()); }
		});

		progressBar->SetStatus("Rendering thumbnails");
		progressBar->SetProgress((float(totalFiles-files.size())/float(totalFiles))*100.0f);
		componentsQueue.push_back(saveButton);
		fileX++;
	}
	else if(componentsQueue.size())
	{
		for(std::vector<ui::Component*>::iterator iter = componentsQueue.begin(), end = componentsQueue.end(); iter != end; ++iter)
		{
			components.push_back(*iter);
			itemList->AddChild(*iter);
		}
		componentsQueue.clear();
		itemList->InnerSize.Y = (buttonHeight+(buttonPadding*2))*(fileY+1);
		if(!componentsQueue.size())
			progressBar->Visible = false;
	}
}

void FileBrowserActivity::OnDraw()
{
	Graphics * g = GetGraphics();

	//Window Background+Outline
	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);
}

FileBrowserActivity::~FileBrowserActivity()
{
	cleanup();
}
