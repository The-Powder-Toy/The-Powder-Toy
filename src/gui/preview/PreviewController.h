#ifndef PREVIEWCONTROLLER_H_
#define PREVIEWCONTROLLER_H_

#include "gui/preview/PreviewModel.h"
#include "gui/preview/PreviewView.h"
#include "Controller.h"
#include "client/SaveInfo.h"
#include "client/ClientListener.h"

class LoginController;
class PreviewModel;
class PreviewView;
class PreviewController: public ClientListener {
	int saveId;
	int saveDate;
	PreviewModel * previewModel;
	PreviewView * previewView;
	LoginController * loginWindow;
	ControllerCallback * callback;
public:
	virtual void NotifyAuthUserChanged(Client * sender);
	inline int SaveID() { return saveId; };

	bool HasExited;
	PreviewController(int saveID, ControllerCallback * callback);
	PreviewController(int saveID, int saveDate, ControllerCallback * callback);
	void Exit();
	void DoOpen();
	void OpenInBrowser();
	void Report(std::string message);
	void ShowLogin();
	bool GetDoOpen();
	SaveInfo * GetSave();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();
	bool SubmitComment(std::string comment);

	void NextCommentPage();
	void PrevCommentPage();

	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
