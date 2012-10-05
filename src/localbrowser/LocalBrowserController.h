/*
 * StampsController.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef STAMPSCONTROLLER_H_
#define STAMPSCONTROLLER_H_

#include "Controller.h"
#include "LocalBrowserView.h"
#include "client/SaveInfo.h"

class LocalBrowserView;
class LocalBrowserModel;
class LocalBrowserController {
	ControllerCallback * callback;
	LocalBrowserView * browserView;
	LocalBrowserModel * browserModel;
public:
	bool HasDone;
	LocalBrowserController(ControllerCallback * callback);
	LocalBrowserView * GetView() {return browserView;}
	SaveFile * GetSave();
	void RemoveSelected();
	void removeSelectedC();
	void ClearSelection();
	void Selected(std::string stampID, bool selected);
	void RescanStamps();
	void rescanStampsC();
	void RefreshSavesList();
	void OpenSave(SaveFile * stamp);
	void SetStamp();
	void NextPage();
	void PrevPage();
	void Update();
	void Exit();
	virtual ~LocalBrowserController();
};

#endif /* STAMPSCONTROLLER_H_ */
