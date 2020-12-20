#ifndef STAMPSCONTROLLER_H_
#define STAMPSCONTROLLER_H_
#include "Config.h"

#include "common/String.h"

#include <functional>

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
	SaveFile * GetSave();
	void RemoveSelected();
	void removeSelectedC();
	void ClearSelection();
	void Selected(ByteString stampID, bool selected);
	void RescanStamps();
	void rescanStampsC();
	void RefreshSavesList();
	void OpenSave(SaveFile * stamp);
	bool GetMoveToFront();
	void SetMoveToFront(bool move);
	void SetPage(int page);
	void SetPageRelative(int offset);
	void Update();
	void Exit();
	virtual ~LocalBrowserController();
};

#endif /* STAMPSCONTROLLER_H_ */
