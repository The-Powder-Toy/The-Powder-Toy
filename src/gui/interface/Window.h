#ifndef WINDOW_H
#define WINDOW_H

#include <vector>
#include "gui/interface/Point.h"
#include "Engine.h"

namespace ui
{

enum ChromeStyle
{
	None, Title, Resizable
};
//class State;
	class Engine;
	class Component;
	class Button;

	/* class State
	 *
	 * A UI state. Contains all components.
	 */
	class Window
	{
	public:
		Point Position;
		Point Size;

		Window(Point _position, Point _size);
		virtual ~Window();

		void SetOkayButton(ui::Button * button) { okayButton = button; }
		void SetCancelButton(ui::Button * button) { cancelButton = button; }

		bool AllowExclusiveDrawing; //false will not call draw on objects outside of bounds

		// Add Component to state
		void AddComponent(Component* c);

		// Get the number of components this state has.
		unsigned GetComponentCount();

		// Get component by index. (See GetComponentCount())
		Component* GetComponent(unsigned idx);

		// Remove a component from state. NOTE: This DOES NOT free component from memory.
		void RemoveComponent(Component* c);

		// Remove a component from state. NOTE: This WILL free component from memory.
		void RemoveComponent(unsigned idx);

		virtual void ToolTip(Component * sender, ui::Point mousePosition, std::string toolTip) {}

		virtual void DoInitialized();
		virtual void DoExit();
		virtual void DoTick(float dt);
		virtual void DoDraw();
		virtual void DoFocus();
		virtual void DoBlur();

		virtual void DoMouseMove(int x, int y, int dx, int dy);
		virtual void DoMouseDown(int x, int y, unsigned button);
		virtual void DoMouseUp(int x, int y, unsigned button);
		virtual void DoMouseWheel(int x, int y, int d);
		virtual void DoKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		virtual void DoKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);

		//Sets halt and destroy, this causes the Windows to stop sending events and remove itself.
		void SelfDestruct();
		void Halt();

		bool IsFocused(const Component* c) const;
		void FocusComponent(Component* c);

		void* UserData;

		enum OkayMethod { Enter, OkayButton };
		enum ExitMethod { MouseOutside, Escape, ExitButton };

	protected:
		ui::Button * okayButton;
		ui::Button * cancelButton;

		virtual void OnInitialized() {}
		virtual void OnExit() {}
		virtual void OnTick(float dt) {}
		virtual void OnDraw() {}
		virtual void OnFocus() {}
		virtual void OnBlur() {}

		virtual void OnTryExit(ExitMethod); 
		virtual void OnTryOkay(OkayMethod);

		virtual void OnMouseMove(int x, int y, int dx, int dy) {}
		virtual void OnMouseDown(int x, int y, unsigned button) {}
		virtual void OnMouseUp(int x, int y, unsigned button) {}
		virtual void OnMouseWheel(int x, int y, int d) {}
		virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) {}
		virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) {}
		std::vector<Component*> Components;
		Component* focusedComponent_;
		ChromeStyle chrome;

		//These controls allow a component to call the destruction of the Window inside an event (called by the Window)
		void finalise();
		bool halt;
		bool destruct;
		bool stop;
#ifdef DEBUG
		bool debugMode;
#endif

	};


/*class Window : public State
{
private:
	ChromeStyle chrome;
public:
	Window(Point _position, Point _size);
	Point Position;
	Point Size;

	virtual void DoTick(float dt);
	virtual void DoDraw();

	virtual void DoMouseMove(int x, int y, int dx, int dy);
	virtual void DoMouseDown(int x, int y, unsigned button);
	virtual void DoMouseUp(int x, int y, unsigned button);
	virtual void DoMouseWheel(int x, int y, int d);
};*/
}
#endif // WINDOW_H
