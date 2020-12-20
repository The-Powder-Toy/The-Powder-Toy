#ifndef AVATARBUTTON_H_
#define AVATARBUTTON_H_

#include "common/String.h"

#include "Component.h"
#include "graphics/Graphics.h"
#include "gui/interface/Colour.h"
#include "client/http/AvatarRequest.h"
#include "client/http/RequestMonitor.h"

#include <memory>
#include <functional>

namespace ui
{
class AvatarButton : public Component, public http::RequestMonitor<http::AvatarRequest>
{
	std::unique_ptr<VideoBuffer> avatar;
	ByteString name;
	bool tried;

	struct AvatarButtonAction
	{
		std::function<void ()> action;
	};
	AvatarButtonAction actionCallback;

public:
	AvatarButton(Point position, Point size, ByteString username);
	virtual ~AvatarButton() = default;

	void OnMouseClick(int x, int y, unsigned int button) override;
	void OnMouseUnclick(int x, int y, unsigned int button) override;

	void OnMouseEnter(int x, int y) override;
	void OnMouseLeave(int x, int y) override;

	void OnContextMenuAction(int item) override;

	void Draw(const Point& screenPos) override;
	void Tick(float dt) override;

	void OnResponse(std::unique_ptr<VideoBuffer> avatar) override;

	void DoAction();

	void SetUsername(ByteString username) { name = username; }
	ByteString GetUsername() { return name; }
	inline void SetActionCallback(AvatarButtonAction const &action) { actionCallback = action; };
protected:
	bool isMouseInside, isButtonDown;
};
}
#endif /* AVATARBUTTON_H_ */

