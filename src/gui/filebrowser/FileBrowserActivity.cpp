#include "FileBrowserActivity.h"

#include "client/GameSave.h"
#include "client/SaveFile.h"
#include "common/platform/Platform.h"
#include "graphics/Graphics.h"
#include "gui/Style.h"
#include "tasks/Task.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/TextPrompt.h"
#include "gui/interface/Label.h"
#include "gui/interface/ProgressBar.h"
#include "gui/interface/SaveButton.h"
#include "gui/interface/ScrollPanel.h"
#include "gui/interface/Textbox.h"

#include "Config.h"
#include <algorithm>

//Currently, reading is done on another thread, we can't render outside the main thread due to some bullshit with OpenGL
class LoadFilesTask: public Task
{
	ByteString directory;
	ByteString search;
	std::vector<std::unique_ptr<SaveFile>> saveFiles;

	void before() override
	{

	}

	void after() override
	{

	}

	bool doWork() override
	{
		std::vector<ByteString> files = Platform::DirectorySearch(directory, search, { ".cps" });
		std::sort(files.rbegin(), files.rend(), [](ByteString a, ByteString b) { return a.ToLower() > b.ToLower(); });

		notifyProgress(-1);
		for(std::vector<ByteString>::iterator iter = files.begin(), end = files.end(); iter != end; ++iter)
		{
			auto saveFile = std::make_unique<SaveFile>(directory + *iter, true);

			ByteString filename = (*iter).SplitFromEndBy(PATH_SEP_CHAR).After();
			filename = filename.SplitFromEndBy('.').Before();
			saveFile->SetDisplayName(filename.FromUtf8());
			saveFiles.push_back(std::move(saveFile));
		}
		return true;
	}

public:
	std::vector<std::unique_ptr<SaveFile>> TakeSaveFiles()
	{
		return std::move(saveFiles);
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
	hasQueuedSearch(false),
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
	if (loadFiles)
	{
		hasQueuedSearch = true;
		queuedSearch = search;
	}
	else
	{
		loadDirectory(directory, search);
	}
}

void FileBrowserActivity::SelectSave(int index)
{
	if (onSelected)
	{
		auto file = std::move(files[index]);
		files.clear();
		onSelected(std::move(file));
	}
	Exit();
}

void FileBrowserActivity::DeleteSave(int index)
{
	String deleteMessage = "Are you sure you want to delete " + files[index]->GetDisplayName() + ".cps?";
	new ConfirmPrompt("Delete Save", deleteMessage, { [this, index]() {
		auto &file = files[index];
		Platform::RemoveFile(file->GetName());
		loadDirectory(directory, "");
	} });
}

void FileBrowserActivity::RenameSave(int index)
{
	new TextPrompt("Rename", "Change save name", files[index]->GetDisplayName(), "", 0, { [this, index](const String &input) {
		auto &file = files[index];
		auto newName = input.ToUtf8();
		if (newName.length())
		{
			newName = ByteString::Build(directory, PATH_SEP_CHAR, newName, ".cps");
			if (!Platform::RenameFile(file->GetName(), newName, false))
			{
				new ErrorMessage("Error", "Could not rename file");
			}
			else
			{
				loadDirectory(directory, "");
			}
		}
		else
		{
			new ErrorMessage("Error", "No save name given");
		}
	} });
}

void FileBrowserActivity::cleanup()
{
	for (auto comp : componentsQueue)
	{
		delete comp;
	}
	componentsQueue.clear();

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
	files = ((LoadFilesTask*)task)->TakeSaveFiles();
	createButtons = true;
	totalFiles = files.size();
	delete loadFiles;
	loadFiles = nullptr;
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

	if (hasQueuedSearch)
	{
		hasQueuedSearch = false;
		loadDirectory(directory, queuedSearch);
	}
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

void FileBrowserActivity::OnTick()
{
	if(loadFiles)
		loadFiles->Poll();

	if (createButtons)
	{
		createButtons = false;
		for (auto i = 0; i < int(files.size()); ++i)
		{
			auto &saveFile = files[i];
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
							saveFile.get());
			saveButton->AddContextMenu(1);
			saveButton->Tick();
			saveButton->SetActionCallback({
				[this, i] { SelectSave(i); },
				[this, i] { RenameSave(i); },
				[this, i] { DeleteSave(i); }
			});

			progressBar->SetStatus("Rendering thumbnails");
			progressBar->SetProgress(totalFiles ? (totalFiles - files.size()) * 100 / totalFiles : 0);
			componentsQueue.push_back(saveButton);
			fileX++;
		}
	}
	if(componentsQueue.size())
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
	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xFFFFFF_rgb);
}

FileBrowserActivity::~FileBrowserActivity()
{
	cleanup();
}
