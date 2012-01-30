/*
 * SSaveController.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef SSAVECONTROLLER_H_
#define SSAVECONTROLLER_H_

#include "SSaveModel.h"
#include "SSaveView.h"
#include "Controller.h"
#include "search/Save.h"

class SSaveView;
class SSaveModel;
class SSaveController {
	SSaveView * ssaveView;
	SSaveModel * ssaveModel;
	ControllerCallback * callback;
public:
	bool HasExited;
	SSaveController(ControllerCallback * callback, Save save);
	Save * GetSave();
	bool GetSaveUploaded();
	void Exit();
	void Update();
	void UploadSave(std::string saveName, std::string saveDescription, bool publish);
	SSaveView * GetView() { return ssaveView; }
	virtual ~SSaveController();
};

#endif /* SSAVECONTROLLER_H_ */
