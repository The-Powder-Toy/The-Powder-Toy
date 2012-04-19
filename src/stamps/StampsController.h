/*
 * StampsController.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef STAMPSCONTROLLER_H_
#define STAMPSCONTROLLER_H_

#include "Controller.h"
#include "StampsView.h"
#include "search/Save.h"

class StampsView;
class StampsModel;
class StampsController {
	ControllerCallback * callback;
	StampsView * stampsView;
	StampsModel * stampsModel;
public:
	bool HasDone;
	StampsController(ControllerCallback * callback);
	StampsView * GetView() {return stampsView;}
	Save * GetStamp();
	void RemoveSelected();
	void removeSelectedC();
	void ClearSelection();
	void Selected(std::string stampID, bool selected);
	void OpenStamp(Save * stamp);
	void SetStamp();
	void NextPage();
	void PrevPage();
	void Update();
	void Exit();
	virtual ~StampsController();
};

#endif /* STAMPSCONTROLLER_H_ */
