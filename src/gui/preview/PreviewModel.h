#ifndef PREVIEWMODEL_H
#define PREVIEWMODEL_H

#include <vector>
#include <iostream>
#include "PreviewView.h"
#include "client/SaveInfo.h"
#include "gui/preview/Comment.h"
#include "gui/search/Thumbnail.h"
#include "client/Download.h"

using namespace std;

class PreviewView;
class PreviewModel {
	bool doOpen;
	bool canOpen;
	vector<PreviewView*> observers;
	SaveInfo * saveInfo;
	std::vector<unsigned char> * saveData;
	std::vector<SaveComment*> * saveComments;
	void notifySaveChanged();
	void notifySaveCommentsChanged();
	void notifyCommentsPageChanged();
	void notifyCommentBoxEnabledChanged();

	Download * saveDataDownload;
	Download * saveInfoDownload;
	Download * commentsDownload;
	int saveID;
	int saveDate;

	bool commentBoxEnabled;
	bool commentsLoaded;
	int commentsTotal;
	int commentsPageNumber;

public:
	PreviewModel();
	SaveInfo * GetSaveInfo();
	std::vector<SaveComment*> * GetComments();

	bool GetCommentBoxEnabled();
	void SetCommentBoxEnabled(bool enabledState);

	bool GetCommentsLoaded();
	int GetCommentsPageNum();
	int GetCommentsPageCount();
	void UpdateComments(int pageNumber);
	void CommentAdded();

	void AddObserver(PreviewView * observer);
	void UpdateSave(int saveID, int saveDate);
	void SetFavourite(bool favourite);
	bool GetDoOpen();
	bool GetCanOpen();
	void SetDoOpen(bool doOpen);
	void Update();
	void ClearComments();
	void OnSaveReady();
	bool ParseSaveInfo(char * saveInfoResponse);
	bool ParseComments(char * commentsResponse);
	virtual ~PreviewModel();
};

#endif /* PREVIEWMODEL_H */
