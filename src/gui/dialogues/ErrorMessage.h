#ifndef ERRORMESSAGE_H_
#define ERRORMESSAGE_H_

#include "gui/interface/Window.h"

class ErrorMessageCallback;
class ErrorMessage: public ui::Window {
	ErrorMessageCallback * callback;
public:
	ErrorMessage(std::string title, std::string message, ErrorMessageCallback * callback_ = nullptr);
	static void Blocking(std::string title, std::string message);
	void OnDraw() override;
	~ErrorMessage() override;
};

class ErrorMessageCallback
{
	public:
		virtual void DismissCallback() {}
		virtual ~ErrorMessageCallback() {}
};

#endif /* ERRORMESSAGE_H_ */
