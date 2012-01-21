#pragma once

#include <stack>
#include <SDL/SDL.h>
#include "Singleton.h"
#include "Platform.h"
#include "Graphics.h"
#include "Window.h"

namespace ui
{
	class Window;

	/* class Engine
	 * 
	 * Controls the User Interface.
	 * Send user inputs to the Engine and the appropriate controls and components will interact.
	 */
	class Engine: public Singleton<Engine>
	{
	public:
		Engine();
		~Engine();

		void ShowWindow(Window * window);
		void CloseWindow();

		void onMouseMove(int x, int y);
		void onMouseClick(int x, int y, unsigned button);
		void onMouseUnclick(int x, int y, unsigned button);
		void onMouseWheel(int x, int y, int delta);
		void onKeyPress(int key, bool shift, bool ctrl, bool alt);
		void onKeyRelease(int key, bool shift, bool ctrl, bool alt);
		void onResize(int newWidth, int newHeight);
		void onClose();

		void Begin(int width, int height);
		inline bool Running() { return running_; }
		void Exit();

		void Tick(float dt);
		void Draw();

		inline int GetMouseX() { return mousex_; }
		inline int GetMouseY() { return mousey_; }
		inline int GetWidth() { return width_; }
		inline int GetHeight() { return height_; }

		inline void SetSize(int width, int height);
		
		//void SetState(Window* state);
		//inline State* GetState() { return state_; }
		inline Window* GetWindow() { return state_; }
		float FpsLimit;
		Graphics * g;
	private:
		pixel * lastBuffer;
		std::stack<pixel*> prevBuffers;
		std::stack<Window*> windows;
		//Window* statequeued_;
		Window* state_;

		bool running_;
		
		int mousex_;
		int mousey_;
		int mousexp_;
		int mouseyp_;
		int width_;
		int height_;
	};

}
