#ifndef STAMPSMODEL_H_
#define STAMPSMODEL_H_

#include <vector>
#include "common/String.h"
#include <cmath>

class SaveFile;

class LocalBrowserView;
class LocalBrowserModel {
	std::vector<ByteString> selected;
	SaveFile * stamp;
	std::vector<ByteString> stampIDs;
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
	std::vector<ByteString> GetSelected() { return selected; }
	void ClearSelected() { selected.clear(); notifySelectedChanged(); }
	void SelectSave(ByteString stampID);
	void DeselectSave(ByteString stampID);
	virtual ~LocalBrowserModel();
};

#endif /* STAMPSMODEL_H_ */
