#pragma once
#include "client/ClientListener.h"
#include "gui/SavePreviewType.h"
#include <functional>
#include <memory>

class VideoBuffer;
class SaveInfo;
class LoginController;
class PreviewModel;
class PreviewView;
class PreviewController: public ClientListener {
	int saveId;
	PreviewModel * previewModel;
	PreviewView * previewView;
	LoginController * loginWindow;
	std::function<void ()> onDone;
public:
	void NotifyAuthUserChanged(Client * sender) override;
	inline int SaveID() { return saveId; }

	bool HasExited;
	PreviewController(int saveID, int saveDate, SavePreviewType savePreviewType, std::function<void ()> onDone, std::unique_ptr<VideoBuffer> thumbnail);
	void Exit();
	void DoOpen();
	void OpenInBrowser();
	void ShowLogin();
	bool GetDoOpen();
	bool GetFromUrl();
	const SaveInfo *GetSaveInfo() const;
	std::unique_ptr<SaveInfo> TakeSaveInfo();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();

	bool NextCommentPage();
	bool PrevCommentPage();
	void CommentAdded();

	virtual ~PreviewController();
};
