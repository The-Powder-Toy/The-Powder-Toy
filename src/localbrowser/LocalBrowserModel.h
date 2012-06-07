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
#include "client/SaveFile.h"

class LocalBrowserView;
class LocalBrowserModel {
	vector<std::string> selected;
	SaveFile * stamp;
	std::vector<std::string> stampIDs;
	std::vector<SaveFile*> savesList;
	std::vector<LocalBrowserView*> observers;
	int currentPage;
	void notifySavesListChanged();
	void notifyPageChanged();
	void notifySelectedChanged();
public:
	LocalBrowserModel();
	int GetPageCount() { return max(1, (int)(ceil(stampIDs.size()/16))); }
	int GetPageNum() { return currentPage; }
	void AddObserver(LocalBrowserView * observer);
	std::vector<SaveFile *> GetSavesList();
	void UpdateSavesList(int pageNumber);
	SaveFile * GetSave();
	void SetSave(SaveFile * newStamp);
	vector<std::string> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(std::string stampID);
	void DeselectSave(std::string stampID);
	virtual ~LocalBrowserModel();
};

#endif /* STAMPSMODEL_H_ */
