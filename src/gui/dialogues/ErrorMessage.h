#ifndef ERRORMESSAGE_H_
#define ERRORMESSAGE_H_

#include "gui/interface/Window.h"

class ErrorMessageCallback;
class ErrorMessage: public ui::Window {
	ErrorMessageCallback * callback;
public:
	ErrorMessage(String title, String message, ErrorMessageCallback * callback_ = NULL);
	static void Blocking(String title, String message);
	void OnDraw() override;
	virtual ~ErrorMessage();
};

class ErrorMessageCallback
{
	public:
		virtual void DismissCallback() {}
		virtual ~ErrorMessageCallback() {}
};

#endif /* ERRORMESSAGE_H_ */
