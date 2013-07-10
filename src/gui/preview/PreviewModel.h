#ifndef PREVIEWMODEL_H
#define PREVIEWMODEL_H

#include <vector>
#include <iostream>
#include <pthread.h>
#undef GetUserName //God dammit microsoft!
#include "PreviewView.h"
#include "client/SaveInfo.h"
#include "gui/preview/Comment.h"
#include "gui/search/Thumbnail.h"

using namespace std;

struct SaveData
{
	SaveData(unsigned char * data_, int len):
		data(data_),
		length(len)
	{
	}
	unsigned char * data;
	int length;
};

struct threadInfo {
	threadInfo(int saveID_, int saveDate_):
		threadFinished(true),
		previewExited(false),
		saveID(saveID_),
		saveDate(saveDate_)
	{
	}
	bool threadFinished;
	bool previewExited;
	int saveID;
	int saveDate;
};

class PreviewView;
class PreviewModel {
	bool doOpen;
	bool commentBoxEnabled;
	vector<PreviewView*> observers;
	SaveInfo * save;
	SaveData *saveData;
	std::vector<SaveComment*> * saveComments;
	void notifySaveChanged();
	void notifySaveCommentsChanged();
	void notifyCommentsPageChanged();
	void notifyCommentBoxEnabledChanged();

	//Background retrieval
	int tSaveID;
	int tSaveDate;

	//
	bool commentsLoaded;
	int commentsTotal;
	int commentsPageNumber;

	threadInfo * updateSaveDataInfo;
	pthread_t updateSaveDataThread;
	static void * updateSaveDataT(void * obj);

	threadInfo * updateSaveInfoInfo;
	pthread_t updateSaveInfoThread;
	static void * updateSaveInfoT(void * obj);

	threadInfo * updateSaveCommentsInfo;
	pthread_t updateSaveCommentsThread;
	static void * updateSaveCommentsT(void * obj);
public:
	PreviewModel();
	SaveInfo * GetSave();
	std::vector<SaveComment*> * GetComments();

	bool GetCommentBoxEnabled();
	void SetCommentBoxEnabled(bool enabledState);

	bool GetCommentsLoaded();
	int GetCommentsPageNum();
	int GetCommentsPageCount();
	void UpdateComments(int pageNumber);

	void AddObserver(PreviewView * observer);
	void UpdateSave(int saveID, int saveDate);
	void SetFavourite(bool favourite);
	bool GetDoOpen();
	void SetDoOpen(bool doOpen);
	void Update();
	virtual ~PreviewModel();
};

#endif /* PREVIEWMODEL_H */
