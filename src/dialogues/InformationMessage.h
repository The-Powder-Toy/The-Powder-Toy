#ifndef INFORMATIONMESSAGE_H_
#define INFORMATIONMESSAGE_H_

#include "interface/Window.h"

class InformationMessage: public ui::Window {
public:
	InformationMessage(std::string title, std::string message);
	virtual void OnDraw();
	virtual ~InformationMessage();
};

#endif /* INFORMATIONMESSAGE_H_ */
