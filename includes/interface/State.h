#pragma once

#include <vector>

#include "Engine.h"
#include "Component.h"
#include "Platform.h"

namespace ui
{
	class Engine;
	class Component;
	
	/* class State
	 * 
	 * A UI state. Contains all components.
	 */
	class State
	{
	public:
		State();
		virtual ~State();

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
		
		void DoInitialized();
		void DoExit();
		void DoTick(float dt);
		void DoDraw();

		void DoMouseMove(int x, int y, int dx, int dy);
		void DoMouseDown(int x, int y, unsigned button);
		void DoMouseUp(int x, int y, unsigned button);
		void DoMouseWheel(int x, int y, int d);
		void DoKeyPress(int key, bool shift, bool ctrl, bool alt);
		void DoKeyRelease(int key, bool shift, bool ctrl, bool alt);

		bool IsFocused(const Component* c) const;
		void FocusComponent(Component* c);

		void* UserData;

	protected:
		virtual void OnInitialized() {}
		virtual void OnExit() {}
		virtual void OnTick(float dt) {}
		virtual void OnDraw() {}

		virtual void OnMouseMove(int x, int y, int dx, int dy) {}
		virtual void OnMouseDown(int x, int y, unsigned button) {}
		virtual void OnMouseUp(int x, int y, unsigned button) {}
		virtual void OnMouseWheel(int x, int y, int d) {}
		virtual void OnKeyPress(int key, bool shift, bool ctrl, bool alt) {}
		virtual void OnKeyRelease(int key, bool shift, bool ctrl, bool alt) {}

	private:
		std::vector<Component*> Components;
		Component* focusedComponent_;

	};

}
