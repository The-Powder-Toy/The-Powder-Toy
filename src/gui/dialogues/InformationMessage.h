#ifndef INFORMATIONMESSAGE_H_
#define INFORMATIONMESSAGE_H_

#include "gui/interface/Window.h"

class InformationMessage: public ui::Window {
public:
	InformationMessage(String title, String message, bool large);
	void OnDraw() override;
	virtual ~InformationMessage();
};

#endif /* INFORMATIONMESSAGE_H_ */
