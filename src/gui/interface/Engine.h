#pragma once
#include <memory>
#include <optional>
#include <stack>
#include "common/String.h"
#include "common/ExplicitSingleton.h"
#include "graphics/Pixel.h"
#include "gui/interface/Point.h"
#include "gui/WindowFrameOps.h"
#include <climits>
#include "FpsLimit.h"

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
		void CloseWindowAndEverythingAbove(Window *window);

		void initialMouse(int x, int y);
		void onMouseMove(int x, int y);
		void onMouseDown(int x, int y, unsigned button);
		void onMouseUp(int x, int y, unsigned button);
		void onMouseWheel(int x, int y, int delta);
		void onKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		void onKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
		void onTextInput(String text);
		void onTextEditing(String text, int start);
		void onClose();
		void onFileDrop(ByteString filename);

		void Begin();
		inline bool Running() { return running_; }
		inline long unsigned int LastTick() { return lastTick; }
		void Exit();
		void ConfirmExit();

		void SetDrawingFrequencyLimit(DrawLimit limit) {drawingFrequencyLimit = limit;}
		inline DrawLimit GetDrawingFrequencyLimit() const {return drawingFrequencyLimit;}
		std::optional<int> GetEffectiveDrawCap() const;
		void SetFastQuit(bool fastquit) { FastQuit = fastquit; }
		inline bool GetFastQuit() {return FastQuit; }
		void SetGlobalQuit(bool newGlobalQuit) { GlobalQuit = newGlobalQuit; }
		inline bool GetGlobalQuit() {return GlobalQuit; }

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

		void SetFpsLimit(FpsLimit newFpsLimit);
		FpsLimit GetFpsLimit() const
		{
			return fpsLimit;
		}

		DrawLimit drawingFrequencyLimit;
		Graphics * g;
		bool GraveExitsConsole;

		bool confirmingExit = false;

		unsigned int FrameIndex;
	private:
		FpsLimit fpsLimit;

		bool textInput = false;
		int lastTextEditingStart = INT_MAX;

		float dt;
		float fps;
		std::deque<Window*> windows;
		std::stack<Point> mousePositions;
		//Window* statequeued_;
		Window* state_;
		Point windowTargetPosition;
		bool ignoreEvents = false;
		std::optional<int> refreshRate;

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
		bool FastQuit;
		bool GlobalQuit;

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
		bool TouchUI = false;
		WindowFrameOps windowFrameOps;

		void SetScale              (int newScale               ) { windowFrameOps.scale               = newScale;               }
		void SetFullscreen         (bool newFullscreen         ) { windowFrameOps.fullscreen          = newFullscreen;          }
		void SetChangeResolution   (bool setChangeResolution   ) { windowFrameOps.changeResolution    = setChangeResolution;    }
		void SetForceIntegerScaling(bool newForceIntegerScaling) { windowFrameOps.forceIntegerScaling = newForceIntegerScaling; }
		void SetResizable          (bool newResizable          ) { windowFrameOps.resizable           = newResizable;           }
		void SetBlurryScaling      (bool newBlurryScaling      ) { windowFrameOps.blurryScaling       = newBlurryScaling;       }
		int  GetScale              () const { return windowFrameOps.scale;               }
		bool GetFullscreen         () const { return windowFrameOps.fullscreen;          }
		bool GetChangeResolution   () const { return windowFrameOps.changeResolution;    }
		bool GetForceIntegerScaling() const { return windowFrameOps.forceIntegerScaling; }
		bool GetResizable          () const { return windowFrameOps.resizable;           }
		bool GetBlurryScaling      () const { return windowFrameOps.blurryScaling;       }

		std::optional<int> GetRefreshRate() const
		{
			return refreshRate;
		}

		void SetRefreshRate(std::optional<int> newRefreshRate)
		{
			refreshRate = newRefreshRate;
		}
	};
}
