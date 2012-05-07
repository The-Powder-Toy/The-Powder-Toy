/*
 * StampsModel.h
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#ifndef STAMPSMODEL_H_
#define STAMPSMODEL_H_

#include <vector>
#include <string>
#include <cmath>
#include "search/Save.h"

class StampsView;
class StampsModel {
	vector<std::string> selected;
	Save * stamp;
	std::vector<std::string> stampIDs;
	std::vector<Save*> stampsList;
	std::vector<StampsView*> observers;
	int currentPage;
	void notifyStampsListChanged();
	void notifyPageChanged();
	void notifySelectedChanged();
public:
	StampsModel();
	int GetPageCount() { return max(1, (int)(ceil(stampIDs.size()/16))); }
	int GetPageNum() { return currentPage; }
	void AddObserver(StampsView * observer);
	std::vector<Save *> GetStampsList();
	void UpdateStampsList(int pageNumber);
	Save * GetStamp();
	void SetStamp(Save * newStamp);
	vector<std::string> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectStamp(std::string stampID);
	void DeselectStamp(std::string stampID);
	virtual ~StampsModel();
};

#endif /* STAMPSMODEL_H_ */
