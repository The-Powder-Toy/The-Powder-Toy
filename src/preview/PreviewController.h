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
	PreviewModel * previewModel;
	PreviewView * previewView;
	ControllerCallback * callback;
public:
	bool HasExited;
	PreviewController(int saveID, ControllerCallback * callback);
	void Exit();
	void DoOpen();
	bool GetDoOpen();
	Save * GetSave();
	PreviewView * GetView() { return previewView; }
	void Update();
	virtual ~PreviewController();
};

#endif /* PREVIEWCONTROLLER_H_ */
