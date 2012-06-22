/*
 * PreviewController.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWCONTROLLER_H_
#define PREVIEWCONTROLLER_H_

#include "preview/PreviewModel.h"
#include "preview/PreviewView.h"
#include "Controller.h"
#include "client/SaveInfo.h"

class PreviewModel;
class PreviewView;
class PreviewController {
	int saveId;
	PreviewModel * previewModel;
	PreviewView * previewView;
	ControllerCallback * callback;
public:
	bool HasExited;
	PreviewController(int saveID, ControllerCallback * callback);
	void Exit();
	void DoOpen();
	void OpenInBrowser();
	void Report(std::string message);
	bool GetDoOpen();
	SaveInfo * GetSave();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();

	void NextCommentPage();
	void PrevCommentPage();

	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
