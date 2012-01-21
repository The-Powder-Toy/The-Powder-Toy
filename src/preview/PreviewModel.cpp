/*
 * PreviewModel.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewModel.h"

PreviewModel::PreviewModel():
	save(NULL)
{
	// TODO Auto-generated constructor stub

}

void PreviewModel::UpdateSave(int saveID)
{

}

void PreviewModel::AddObserver(PreviewView * observer) {
	observers.push_back(observer);
}

PreviewModel::~PreviewModel() {
	// TODO Auto-generated destructor stub
}

