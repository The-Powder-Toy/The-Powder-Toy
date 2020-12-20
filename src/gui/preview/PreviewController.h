#ifndef PREVIEWCONTROLLER_H_
#define PREVIEWCONTROLLER_H_
#include "Config.h"

#include "client/ClientListener.h"

#include <functional>

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
	PreviewController(int saveID, int saveDate, bool instant, std::function<void ()> onDone = nullptr);
	void Exit();
	void DoOpen();
	void OpenInBrowser();
	void Report(String message);
	void ShowLogin();
	bool GetDoOpen();
	SaveInfo * GetSaveInfo();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();
	bool SubmitComment(String comment);

	bool NextCommentPage();
	bool PrevCommentPage();

	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
