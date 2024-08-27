#pragma once
#include "common/String.h"
#include <functional>
#include <memory>

class SaveFile;
class LocalBrowserView;
class LocalBrowserModel;
class LocalBrowserController {
	LocalBrowserView * browserView;
	LocalBrowserModel * browserModel;
	std::function<void ()> onDone;
public:
	bool HasDone;
	LocalBrowserController(std::function<void ()> onDone = nullptr);
	LocalBrowserView * GetView() {return browserView;}
	std::unique_ptr<SaveFile> TakeSave();
	void RemoveSelected();
	void removeSelectedC();
	void RenameSelected();
	void ClearSelection();
	void Selected(ByteString stampID, bool selected);
	void RescanStamps();
	void RefreshSavesList();
	void OpenSave(int index);
	bool GetMoveToFront();
	void SetMoveToFront(bool move);
	void SetPage(int page);
	void SetPageRelative(int offset);
	void Update();
	void Exit();
	virtual ~LocalBrowserController();
};
