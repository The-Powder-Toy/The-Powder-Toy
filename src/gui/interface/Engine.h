#pragma once

#include <stack>
#include "common/Singleton.h"
#include "graphics/Graphics.h"
#include "Window.h"
#include "PowderToy.h"

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
		int CloseWindow();

		void onMouseMove(int x, int y);
		void onMouseClick(int x, int y, unsigned button);
		void onMouseUnclick(int x, int y, unsigned button);
		void onMouseWheel(int x, int y, int delta);
		void onKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		void onKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
		void onResize(int newWidth, int newHeight);
		void onClose();

		void Begin(int width, int height);
		inline bool Running() { return running_; }
		inline bool Broken() { return break_; } 
		inline long unsigned int LastTick() { return lastTick; }
		inline void LastTick(long unsigned int tick) { lastTick = tick; }
		void Exit();
		void Break();
		void UnBreak();

		void SetFullscreen(bool fullscreen) { Fullscreen = fullscreen; }
		inline bool GetFullscreen() { return Fullscreen; }
		void SetScale(int scale) { Scale = scale; }
		inline int GetScale() { return Scale; }
		void Set3dDepth(int depth3d) { Depth3d = depth3d; if (Depth3d) SetCursorEnabled(0); else SetCursorEnabled(1);}
		inline int Get3dDepth() { return Depth3d; }
		void SetFastQuit(bool fastquit) { FastQuit = fastquit; }
		inline bool GetFastQuit() {return FastQuit; }

		void Tick();
		void Draw();

		void SetFps(float fps);
		inline float GetFps() { return fps; }
		inline float GetDelta() { return dt; }

		inline int GetMouseButton() { return mouseb_; }
		inline int GetMouseX() { return mousex_; }
		inline int GetMouseY() { return mousey_; }
		inline int GetWidth() { return width_; }
		inline int GetHeight() { return height_; }
		inline int GetMaxWidth() { return maxWidth; }
		inline int GetMaxHeight() { return maxHeight; }

		void SetMaxSize(int width, int height);

		inline void SetSize(int width, int height);
		
		//void SetState(Window* state);
		//inline State* GetState() { return state_; }
		inline Window* GetWindow() { return state_; }
		float FpsLimit;
		Graphics * g;
		int Scale;
		bool Fullscreen;
		int Depth3d;

		unsigned int FrameIndex;
	private:
		float dt;
		float fps;
		pixel * lastBuffer;
		std::stack<pixel*> prevBuffers;
		std::stack<Window*> windows;
		std::stack<Point> mousePositions;
		//Window* statequeued_;
		Window* state_;
		Point windowTargetPosition;
		int windowOpenState;

		bool running_;
		bool break_;
		bool FastQuit;
		
		long unsigned int lastTick;
		int mouseb_;
		int mousex_;
		int mousey_;
		int mousexp_;
		int mouseyp_;
		int width_;
		int height_;

		int maxWidth;
		int maxHeight;
	};

}
