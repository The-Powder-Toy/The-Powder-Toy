#pragma once

#include <string>
#include "common/tpt-compat.h"
#include "Appearance.h"
#include "Point.h"

class Graphics;
namespace ui
{
	class ContextMenu;
	class Window;
	class Panel;

	/* class Component
	 *
	 * An interactive UI component that can be added to a state or an XComponent*.
	 * *See sys::XComponent
	 */
	class Component
	{
	private:
		Window* parentstate_;
		Panel* _parent;
	protected:
		bool drawn;
		ui::Point textPosition;
		ui::Point textSize;
		ui::Point iconPosition;
		ui::ContextMenu * menu;
		Graphics * GetGraphics();
	public:
		Component(Window* parent_state);
		Component(Point position, Point size);
		Component();
		virtual ~Component();

		void* UserData;
		inline Window* const GetParentWindow() const { return parentstate_; }
		bool IsFocused() const;

		void Invalidate() { drawn = false; }

		Point Position;
		Point Size;
		bool Enabled;
		bool Visible;

		ui::Appearance Appearance;
		//virtual void SetAppearance(ui::Appearance);
		//ui::Appearance GetAppearance();
		virtual void TextPosition(std::string);

		void Refresh();

		Point GetScreenPos();

		/* See the parent of this component.
		 * If new_parent is NULL, this component will have no parent. (THIS DOES NOT delete THE COMPONENT. See XComponent::RemoveChild)
		 */
		void SetParentWindow(Window* window);
		void SetParent(Panel* new_parent);

		//Get the parent component.
		inline Panel* const GetParent() const { return _parent; }

		virtual void OnContextMenuAction(int item);

		//UI functions:
		/*
			void Tick(float dt);
			void Draw(const Point& screenPos);

			void OnMouseHover(int localx, int localy);
			void OnMouseMoved(int localx, int localy, int dx, int dy);
			void OnMouseMovedInside(int localx, int localy, int dx, int dy);
			void OnMouseEnter(int localx, int localy);
			void OnMouseLeave(int localx, int localy);
			void OnMouseDown(int x, int y, unsigned int button);
			void OnMouseUp(int x, int y, unsigned int button);
			void OnMouseClick(int localx, int localy, unsigned int button);
			void OnMouseUnclick(int localx, int localy, unsigned int button);
			void OnMouseWheel(int localx, int localy, int d);
			void OnMouseWheelInside(int localx, int localy, int d);
			void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
			void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		*/

		///
		// Called: Every tick.
		// Params:
		//	dt: The change in time.
		///
		virtual void Tick(float dt);

		///
		// Called: When ready to draw.
		// Params:
		// 	None
		///
		virtual void Draw(const Point& screenPos);




		///
		// Called: When the mouse is currently hovering over the item. (Called every tick)
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		///
		virtual void OnMouseHover(int localx, int localy);

		///
		// Called: When the mouse moves.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		// 	dx: Mouse X delta.
		// 	dy: Mouse Y delta.
		///
		virtual void OnMouseMoved(int localx, int localy, int dx, int dy);

		///
		// Called: When the mouse moves.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		// 	dx: Mouse X delta.
		// 	dy: Mouse Y delta.
		///
		virtual void OnMouseMovedInside(int localx, int localy, int dx, int dy);

		///
		// Called: When the mouse moves on top of the item.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		// 	dx: Mouse X delta.
		// 	dy: Mouse Y delta.
		///
		virtual void OnMouseEnter(int localx, int localy);

		///
		// Called: When the mouse leaves the item.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		///
		virtual void OnMouseLeave(int localx, int localy);

		///
		// Called: When a mouse button is pressed.
		// Params:
		// 	x: X position of the mouse.
		// 	y: Y position of the mouse.
		// 	button: The button that is being held down.
		///
		virtual void OnMouseDown(int x, int y, unsigned button);

		///
		// Called: When a mouse button is released.
		// Params:
		// 	x: X position of the mouse.
		// 	y: Y position of the mouse.
		// 	button: The button that is being released.
		///
		virtual void OnMouseUp(int x, int y, unsigned button);

		///
		// Called: When a mouse button is pressed on top of the item.
		// Params:
		// 	x: X position of the mouse.
		// 	y: Y position of the mouse.
		// 	button: The button that is being held down.
		///
		virtual void OnMouseClick(int localx, int localy, unsigned button);

		///
		// Called: When a mouse button is released on top of the item.
		// Params:
		// 	x: X position of the mouse.
		// 	y: Y position of the mouse.
		// 	button: The button that is being released.
		///
		virtual void OnMouseUnclick(int localx, int localy, unsigned button);

		///
		// Called: When the mouse wheel moves/changes.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		// 	d: The mouse wheel movement value.
		///
		virtual void OnMouseWheel(int localx, int localy, int d);

		///
		// Called: When the mouse wheel moves/changes on top of the item.
		// Params:
		//	localx: Local mouse X position.
		//	localy: Local mouse Y position.
		// 	d: The mouse wheel movement value.
		///
		virtual void OnMouseWheelInside(int localx, int localy, int d);

		///
		// Called: When a key is pressed.
		// Params:
		// 	key: The value of the key that is being pressed.
		// 	shift: Shift key is down.
		// 	ctrl: Control key is down.
		// 	alt: Alternate key is down.
		///
		virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);

		///
		// Called: When a key is released.
		// Params:
		// 	key: The value of the key that is being released.
		// 	shift: Shift key is released.
		// 	ctrl: Control key is released.
		// 	alt: Alternate key is released.
		///
		virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	};
}
