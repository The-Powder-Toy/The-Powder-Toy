/*
 * PreviewModel.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWMODEL_H_
#define PREVIEWMODEL_H_

#include <vector>
#include <pthread.h>
#include "PreviewView.h"
#include "search/Save.h"
#include "preview/Comment.h"
#include "search/Thumbnail.h"

using namespace std;

class PreviewView;
class PreviewModel {
	bool doOpen;
	vector<PreviewView*> observers;
	Save * save;
	Thumbnail * savePreview;
	std::vector<Comment*> * saveComments;
	void notifyPreviewChanged();
	void notifySaveChanged();
	void notifySaveCommentsChanged();

	//Background retrieval
	int tSaveID;
	int tSaveDate;

	bool updateSavePreviewWorking;
	volatile bool updateSavePreviewFinished;
	pthread_t updateSavePreviewThread;
	static void * updateSavePreviewTHelper(void * obj);
	void * updateSavePreviewT();

	bool updateSaveInfoWorking;
	volatile bool updateSaveInfoFinished;
	pthread_t updateSaveInfoThread;
	static void * updateSaveInfoTHelper(void * obj);
	void * updateSaveInfoT();

	bool updateSaveCommentsWorking;
	volatile bool updateSaveCommentsFinished;
	pthread_t updateSaveCommentsThread;
	static void * updateSaveCommentsTHelper(void * obj);
	void * updateSaveCommentsT();
public:
	PreviewModel();
	Thumbnail * GetPreview();
	Save * GetSave();
	std::vector<Comment*> * GetComments();
	void AddObserver(PreviewView * observer);
	void UpdateSave(int saveID, int saveDate);
	bool GetDoOpen();
	void SetDoOpen(bool doOpen);
	void Update();
	virtual ~PreviewModel();
};

#endif /* PREVIEWMODEL_H_ */
