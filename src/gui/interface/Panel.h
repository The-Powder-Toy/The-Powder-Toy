#pragma once
#include <vector>

#include "graphics/Pixel.h"
#include "gui/interface/Point.h"
#include "gui/interface/Window.h"
#include "gui/interface/Component.h"

#ifdef OGLI
#include "graphics/OpenGLHeaders.h"
#endif


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

#ifdef OGLI
		GLuint myVid, myVidTex;
#else 
		pixel * myVid;
#endif
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
		
		void Tick(float dt);
		void Draw(const Point& screenPos);
		
		void OnMouseHover(int localx, int localy);
		void OnMouseMoved(int localx, int localy, int dx, int dy);
		void OnMouseMovedInside(int localx, int localy, int dx, int dy);
		void OnMouseEnter(int localx, int localy);
		void OnMouseLeave(int localx, int localy);
		void OnMouseDown(int x, int y, unsigned button);
		void OnMouseUp(int x, int y, unsigned button);
		void OnMouseClick(int localx, int localy, unsigned button);
		void OnMouseUnclick(int localx, int localy, unsigned button);
		void OnMouseWheel(int localx, int localy, int d);
		void OnMouseWheelInside(int localx, int localy, int d);
		void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		
	protected:
		// child components
		std::vector<ui::Component*> children;
		bool mouseInside;
		
		//UI functions:
		/*
			void XTick(float dt);
			void XDraw(const Point& screenPos);
			
			void XOnMouseHover(int localx, int localy);
			void XOnMouseMoved(int localx, int localy, int dx, int dy);
			void XOnMouseMovedInside(int localx, int localy, int dx, int dy);
			void XOnMouseEnter(int localx, int localy);
			void XOnMouseLeave(int localx, int localy);
			void XOnMouseDown(int x, int y, unsigned int button);
			void XOnMouseUp(int x, int y, unsigned int button);
			void XOnMouseClick(int localx, int localy, unsigned int button);
			void XOnMouseUnclick(int localx, int localy, unsigned int button);
			void XOnMouseWheel(int localx, int localy, int d);
			void XOnMouseWheelInside(int localx, int localy, int d);
			void XOnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
			void XOnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		*/
		
		// Overridable. Called by XComponent::Tick()
		virtual void XTick(float dt);
		
		// Overridable. Called by XComponent::Draw()
		virtual void XDraw(const Point& screenPos);
		
		
		// Overridable. Called by XComponent::XOnMouseHover()
		virtual void XOnMouseHover(int localx, int localy);
		
		// Overridable. Called by XComponent::OnMouseMoved()
		virtual void XOnMouseMoved(int localx, int localy, int dx, int dy);
		
		// Overridable. Called by XComponent::OnMouseMovedInside()
		virtual void XOnMouseMovedInside(int localx, int localy, int dx, int dy);
		
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
		
		// Overridable. Called by XComponent::OnMouseUnclick()
		virtual void XOnMouseUnclick(int localx, int localy, unsigned button);
		
		// Overridable. Called by XComponent::OnMouseWheel()
		virtual void XOnMouseWheel(int localx, int localy, int d);
		
		// Overridable. Called by XComponent::OnMouseWheelInside()
		virtual void XOnMouseWheelInside(int localx, int localy, int d);
		
		// Overridable. Called by XComponent::OnKeyPress()
		virtual void XOnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		
		// Overridable. Called by XComponent::OnKeyRelease()
		virtual void XOnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	};

}
