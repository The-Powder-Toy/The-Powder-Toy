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
#include "search/Save.h"

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
	Save * GetSave();
	PreviewView * GetView() { return previewView; }
	void Update();
	void FavouriteSave();
	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
