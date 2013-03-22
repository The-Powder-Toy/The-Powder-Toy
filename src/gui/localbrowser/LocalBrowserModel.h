#ifndef STAMPSMODEL_H_
#define STAMPSMODEL_H_

#include <vector>
#include <string>
#include <cmath>

class SaveFile;

class LocalBrowserView;
class LocalBrowserModel {
	std::vector<std::string> selected;
	SaveFile * stamp;
	std::vector<std::string> stampIDs;
	std::vector<SaveFile*> savesList;
	std::vector<LocalBrowserView*> observers;
	int currentPage;
	bool stampToFront;
	void notifySavesListChanged();
	void notifyPageChanged();
	void notifySelectedChanged();
public:
	LocalBrowserModel();
	int GetPageCount();
	int GetPageNum() { return currentPage; }
	void AddObserver(LocalBrowserView * observer);
	std::vector<SaveFile *> GetSavesList();
	void UpdateSavesList(int pageNumber);
	void RescanStamps();
	SaveFile * GetSave();
	void SetSave(SaveFile * newStamp);
	bool GetMoveToFront();
	void SetMoveToFront(bool move);
	std::vector<std::string> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(std::string stampID);
	void DeselectSave(std::string stampID);
	virtual ~LocalBrowserModel();
};

#endif /* STAMPSMODEL_H_ */
