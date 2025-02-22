#pragma once
#include "graphics/Pixel.h"
#include "gui/interface/Point.h"
#include "gui/interface/Component.h"
#include <vector>

class Graphics;
namespace ui
{
	/* class XComponent
	 *
	 * An eXtension of the Component class.
	 * Adds the ability to have child components.
	 *
	 * See sys::Component
	 */

	class Component;

	class Panel : public Component
	{
	public:
		friend class Component;

		ui::Point InnerSize;
		ui::Point ViewportPosition;

		Panel(Point position, Point size);
		virtual ~Panel();

		/* Add a child component.
		 * Similar to XComponent::SetParent
		 *
		 * If the component is already parented, then this will become the new parent.
		 */
		void AddChild(Component* c);

		// Remove child from component. This DOES NOT free the component from memory.
		void RemoveChild(Component* c);

		// Remove child from component. This WILL free the component from memory unless told otherwise.
		void RemoveChild(unsigned idx, bool freeMem = true);

		//Grab the number of children this component owns.
		int GetChildCount();

		//Get child of this component by index.
		Component* GetChild(unsigned idx);

		void Tick() override;
		void Draw(const Point& screenPos) override;

		void OnMouseHover(int localx, int localy) override;
		void OnMouseMoved(int localx, int localy) override;
		void OnMouseEnter(int localx, int localy) override;
		void OnMouseLeave(int localx, int localy) override;
		void OnMouseDown(int x, int y, unsigned button) override;
		void OnMouseUp(int x, int y, unsigned button) override;
		void OnMouseClick(int localx, int localy, unsigned button) override;
		void OnMouseWheel(int localx, int localy, int d) override;
		void OnMouseWheelInside(int localx, int localy, int d) override;
		void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
		void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;

	protected:
		// child components
		std::vector<ui::Component*> children;

		// Overridable. Called by XComponent::Tick()
		virtual void XTick();

		// Overridable. Called by XComponent::Draw()
		virtual void XDraw(const Point& screenPos);


		// Overridable. Called by XComponent::XOnMouseHover()
		virtual void XOnMouseHover(int localx, int localy);

		// Overridable. Called by XComponent::OnMouseMoved()
		virtual void XOnMouseMoved(int localx, int localy);

		// Overridable. Called by XComponent::OnMouseEnter()
		virtual void XOnMouseEnter(int localx, int localy);

		// Overridable. Called by XComponent::OnMouseLeave()
		virtual void XOnMouseLeave(int localx, int localy);

		// Overridable. Called by XComponent::OnMouseDown()
		virtual void XOnMouseDown(int x, int y, unsigned button);

		// Overridable. Called by XComponent::OnMouseUp()
		virtual void XOnMouseUp(int x, int y, unsigned button);

		// Overridable. Called by XComponent::OnMouseClick()
		virtual void XOnMouseClick(int localx, int localy, unsigned button);

		// Overridable. Called by XComponent::OnMouseWheel()
		virtual void XOnMouseWheel(int localx, int localy, int d);

		// Overridable. Called by XComponent::OnMouseWheelInside()
		virtual void XOnMouseWheelInside(int localx, int localy, int d);

		// Overridable. Called by XComponent::OnKeyPress()
		virtual void XOnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);

		// Overridable. Called by XComponent::OnKeyRelease()
		virtual void XOnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		
		void PropagateMouseMove();
	};

}
