#include "LocalBrowserController.h"

#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"

#include "client/Client.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "tasks/TaskWindow.h"
#include "tasks/Task.h"

#include "Controller.h"

#include "common/tpt-minmax.h"

LocalBrowserController::LocalBrowserController(std::function<void ()> onDone_):
	HasDone(false)
{
	browserModel = new LocalBrowserModel();
	browserView = new LocalBrowserView();
	browserView->AttachController(this);
	browserModel->AddObserver(browserView);

	onDone = onDone_;

	browserModel->UpdateSavesList(1);
}

void LocalBrowserController::OpenSave(SaveFile * save)
{
	browserModel->SetSave(save);
}

SaveFile * LocalBrowserController::GetSave()
{
	return browserModel->GetSave();
}

void LocalBrowserController::RemoveSelected()
{
	StringBuilder desc;
	desc << "Are you sure you want to delete " << browserModel->GetSelected().size() << " stamp";
	if(browserModel->GetSelected().size()>1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt("Delete stamps", desc.Build(), { [this] { removeSelectedC(); } });
}

void LocalBrowserController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		std::vector<ByteString> saves;
		LocalBrowserController * c;
	public:
		RemoveSavesTask(LocalBrowserController * c, std::vector<ByteString> saves_) : c(c) { saves = saves_; }
		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				notifyStatus(String::Build("Deleting stamp [", saves[i].FromUtf8(), "] ..."));
				Client::Ref().DeleteStamp(saves[i]);
				notifyProgress((i + 1) * 100 / saves.size());
			}
			return true;
		}
		void after() override
		{
			Client::Ref().updateStamps();
			c->RefreshSavesList();
		}
	};

	std::vector<ByteString> selected = browserModel->GetSelected();
	new TaskWindow("Removing stamps", new RemoveSavesTask(this, selected));
}

void LocalBrowserController::RescanStamps()
{
	new ConfirmPrompt("Rescan", "Rescanning the stamps folder can find stamps added to the stamps folder or recover stamps when the stamps.def file has been lost or damaged. However, be warned that this will mess up the current sorting order", { [this] { rescanStampsC(); } });
}

void LocalBrowserController::rescanStampsC()
{
	browserModel->RescanStamps();
	browserModel->UpdateSavesList(browserModel->GetPageNum());
}

void LocalBrowserController::RefreshSavesList()
{
	ClearSelection();
	browserModel->UpdateSavesList(browserModel->GetPageNum());
}

void LocalBrowserController::ClearSelection()
{
	browserModel->ClearSelected();
}

void LocalBrowserController::SetPage(int page)
{
	if (page != browserModel->GetPageNum() && page > 0 && page <= browserModel->GetPageCount())
		browserModel->UpdateSavesList(page);
}

void LocalBrowserController::SetPageRelative(int offset)
{
	int page = std::min(std::max(browserModel->GetPageNum() + offset, 1), browserModel->GetPageCount());
	if (page != browserModel->GetPageNum())
		browserModel->UpdateSavesList(page);
}

void LocalBrowserController::Update()
{
	if(browserModel->GetSave())
	{
		Exit();
	}
}

void LocalBrowserController::Selected(ByteString saveName, bool selected)
{
	if(selected)
		browserModel->SelectSave(saveName);
	else
		browserModel->DeselectSave(saveName);
}

bool LocalBrowserController::GetMoveToFront()
{
	return browserModel->GetMoveToFront();
}

void LocalBrowserController::SetMoveToFront(bool move)
{
	browserModel->SetMoveToFront(move);
}

void LocalBrowserController::Exit()
{
	browserView->CloseActiveWindow();
	if (onDone)
		onDone();
	HasDone = true;
}

LocalBrowserController::~LocalBrowserController()
{
	browserView->CloseActiveWindow();
	delete browserModel;
	delete browserView;
}

