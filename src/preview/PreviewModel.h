/*
 * PreviewModel.h
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#ifndef PREVIEWMODEL_H_
#define PREVIEWMODEL_H_

#include <vector>
#include "PreviewView.h"
#include "search/Save.h"
#include "search/Thumbnail.h"

using namespace std;

class PreviewView;
class PreviewModel {
	bool doOpen;
	vector<PreviewView*> observers;
	Save * save;
	Thumbnail * savePreview;
	void notifyPreviewChanged();
	void notifySaveChanged();
public:
	PreviewModel();
	Thumbnail * GetPreview();
	Save * GetSave();
	void AddObserver(PreviewView * observer);
	void UpdateSave(int saveID, int saveDate);
	bool GetDoOpen();
	void SetDoOpen(bool doOpen);
	virtual ~PreviewModel();
};

#endif /* PREVIEWMODEL_H_ */
