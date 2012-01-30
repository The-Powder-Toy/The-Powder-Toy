/*
 * SSaveModel.h
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#ifndef SSAVEMODEL_H_
#define SSAVEMODEL_H_

#include <vector>

#include "SSaveView.h"
#include "search/Save.h"

using namespace std;

class SSaveView;
class SSaveModel {
	vector<SSaveView*> observers;
	Save * save;
	void notifySaveChanged();
	void notifySaveUploadChanged();
	bool saveUploaded;
public:
	SSaveModel();
	void AddObserver(SSaveView * observer);
	void Update();
	Save * GetSave();
	void SetSave(Save * save);
	void UploadSave(std::string saveName, std::string saveDescription, bool publish);
	bool GetSaveUploaded();
	virtual ~SSaveModel();
};

#endif /* SSAVEMODEL_H_ */
