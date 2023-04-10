#pragma once
#include <memory>
#include <stack>
#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include "graphics/Pixel.h"
#include "gui/interface/Point.h"

class Graphics;
namespace ui
{
	class Window;

	/* class Engine
	 *
	 * Controls the User Interface.
	 * Send user inputs to the Engine and the appropriate controls and components will interact.
	 */
	class Engine: public ExplicitSingleton<Engine>
	{
	public:
		Engine();
		~Engine();

		void ShowWindow(Window * window);
		int CloseWindow();

		void initialMouse(int x, int y);
		void onMouseMove(int x, int y);
		void onMouseClick(int x, int y, unsigned button);
		void onMouseUnclick(int x, int y, unsigned button);
		void onMouseWheel(int x, int y, int delta);
		void onKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		void onKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		void onTextInput(String text);
		void onTextEditing(String text, int start);
		void onClose();
		void onFileDrop(ByteString filename);

		void Begin();
		inline bool Running() { return running_; }
		inline bool Broken() { return break_; }
		inline long unsigned int LastTick() { return lastTick; }
		void Exit();
		void ConfirmExit();
		void Break();
		void UnBreak();

		void SetDrawingFrequencyLimit(int limit) {drawingFrequencyLimit = limit;}
		inline int GetDrawingFrequencyLimit() {return drawingFrequencyLimit;}
		void SetFullscreen(bool fullscreen) { Fullscreen = fullscreen; }
		inline bool GetFullscreen() { return Fullscreen; }
		void SetAltFullscreen(bool altFullscreen) { this->altFullscreen = altFullscreen; }
		inline bool GetAltFullscreen() { return altFullscreen; }
		void SetForceIntegerScaling(bool forceIntegerScaling) { this->forceIntegerScaling = forceIntegerScaling; }
		inline bool GetForceIntegerScaling() { return forceIntegerScaling; }
		void SetScale(int scale) { Scale = scale; }
		inline int GetScale() { return Scale; }
		void SetResizable(bool resizable) { this->resizable = resizable; }
		inline bool GetResizable() { return resizable; }
		void SetFastQuit(bool fastquit) { FastQuit = fastquit; }
		inline bool GetFastQuit() {return FastQuit; }

		void Tick();
		void Draw();

		void SetFps(float fps);
		inline float GetFps() { return fps; }

		inline int GetMouseButton() { return mouseb_; }
		inline int GetMouseX() { return mousex_; }
		inline int GetMouseY() { return mousey_; }

		void StartTextInput();
		void StopTextInput();
		void TextInputRect(Point position, Point size);

		//void SetState(Window* state);
		//inline State* GetState() { return state_; }
		inline Window* GetWindow() { return state_; }
		float FpsLimit;
		int drawingFrequencyLimit;
		Graphics * g;
		int Scale;
		bool Fullscreen;

		unsigned int FrameIndex;
	private:
		bool altFullscreen;
		bool forceIntegerScaling = true;
		bool resizable;

		bool textInput = false;

		float dt;
		float fps;
		std::stack<Window*> windows;
		std::stack<Point> mousePositions;
		//Window* statequeued_;
		Window* state_;
		Point windowTargetPosition;
		bool ignoreEvents = false;

		// saved appearances of windows that are in the backround and
		// thus are not currently being redrawn
		struct FrozenGraphics
		{
			int fadeTicks;
			std::unique_ptr<pixel []> screen;
		};
		constexpr static int maxFadeTicks = 20;
		std::stack<FrozenGraphics> frozenGraphics;

		bool running_;
		bool break_;
		bool FastQuit;

		long unsigned int lastTick;
		int mouseb_;
		int mousex_;
		int mousey_;
		int mousexp_;
		int mouseyp_;

		String textEditingBuf;

	public:
		bool MomentumScroll = true;
		bool ShowAvatars = true;
	};

}
