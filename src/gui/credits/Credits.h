#pragma once
#include <vector>
#include "gui/interface/Window.h"

class Credits : public ui::Window
{
	enum CreditSize
	{
		Small,
		Large,
		Half,
	};

	static std::vector<ui::Component *> AddCredit(const String &name, const String &subheader, CreditSize size,
		const ByteString &uri, bool includeAvatar = false, int grayscale = 255);
	static ByteString GetProfileUri(const ByteString &username);
	static ByteString GetTptLabelText(const ByteString &tpt, const ByteString &github);
	static String GetRichLabelText(const ByteString &uri, const String &message);
public:
	Credits();

	void OnTryExit(ExitMethod method) override;
};
