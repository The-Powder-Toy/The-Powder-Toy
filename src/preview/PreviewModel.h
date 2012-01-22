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

using namespace std;

class PreviewView;
class PreviewModel {
	vector<PreviewView*> observers;
	Save * save;
public:
	PreviewModel();
	void AddObserver(PreviewView * observer);
	void UpdateSave(int saveID);
	virtual ~PreviewModel();
};

#endif /* PREVIEWMODEL_H_ */
