#pragma once
#include "common/String.h"
#include <vector>
#include <memory>

namespace http
{
	class Request;
}

class PreviewView;
class SaveInfo;
class SaveComment;
class PreviewModel
{
	bool doOpen;
	bool canOpen;
	std::vector<PreviewView*> observers;
	std::unique_ptr<SaveInfo> saveInfo;
	std::vector<char> * saveData;
	std::vector<SaveComment*> * saveComments;
	void notifySaveChanged();
	void notifySaveCommentsChanged();
	void notifyCommentsPageChanged();
	void notifyCommentBoxEnabledChanged();

	std::unique_ptr<http::Request> saveDataDownload;
	std::unique_ptr<http::Request> saveInfoDownload;
	std::unique_ptr<http::Request> commentsDownload;
	int saveID;
	int saveDate;

	bool commentBoxEnabled;
	bool commentsLoaded;
	int commentsTotal;
	int commentsPageNumber;

public:
	PreviewModel();
	~PreviewModel();

	const SaveInfo *GetSaveInfo() const;
	std::unique_ptr<SaveInfo> TakeSaveInfo();
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
	bool ParseSaveInfo(ByteString &saveInfoResponse);
	bool ParseComments(ByteString &commentsResponse);
};
