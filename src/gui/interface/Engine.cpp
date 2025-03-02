#include "Engine.h"
#include "Config.h"
#include "PowderToySDL.h"
#include "Window.h"
#include "common/platform/Platform.h"
#include "graphics/Graphics.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include <cmath>
#include <cstring>

using namespace ui;

Engine::Engine():
	drawingFrequencyLimit(DrawLimitDisplay{}),
	FrameIndex(0),
	state_(nullptr),
	windowTargetPosition(0, 0),
	FastQuit(1),
	GlobalQuit(true),
	lastTick(Platform::GetTime()),
	mouseb_(0),
	mousex_(0),
	mousey_(0),
	mousexp_(0),
	mouseyp_(0)
{
}

Engine::~Engine()
{
	delete state_;
	//Dispose of any Windows.
	while (!windows.empty())
	{
		delete windows.back();
		windows.pop_back();
	}
}

void Engine::ApplyFpsLimit()
{
	::ApplyFpsLimit();
}

void Engine::Begin()
{
	//engine is now ready
	running_ = true;
}

void Engine::Exit()
{
	onClose();
	running_ = false;
}

void Engine::ConfirmExit()
{
	if (!confirmingExit)
	{
		confirmingExit = true;
		new ConfirmPrompt("You are about to quit", "Are you sure you want to exit the game?", { [] {
			ui::Engine::Ref().Exit();
		}, [this] {
			confirmingExit = false;
		} });
	}
}

void Engine::ShowWindow(Window * window)
{
	CloseWindowAndEverythingAbove(window);
	if (state_)
		ignoreEvents = true;
	if(window->Position.X==-1)
	{
		window->Position.X = (g->Size().X - window->Size.X) / 2;
	}
	if(window->Position.Y==-1)
	{
		window->Position.Y = (g->Size().Y - window->Size.Y) / 2;
	}
	window->Size = window->Size.Min(g->Size());
	window->Position = window->Position.Clamp(RectBetween<int>({0, 0}, g->Size()));
	/*if(window->Position.Y > 0)
	{
		windowTargetPosition = window->Position;
		window->Position = Point(windowTargetPosition.X, height_);
	}*/
	if(state_)
	{
		frozenGraphics.emplace(FrozenGraphics{0, std::make_unique<pixel []>(g->Size().X * g->Size().Y)});
		std::copy_n(g->Data(), g->Size().X * g->Size().Y, frozenGraphics.top().screen.get());

		windows.push_back(state_);
		mousePositions.push(ui::Point(mousex_, mousey_));
	}
	if(state_)
		state_->DoBlur();

	state_ = window;
	ApplyFpsLimit();
}

void Engine::CloseWindowAndEverythingAbove(Window *window)
{
	if (window == state_)
	{
		CloseWindow();
		return;
	}
	auto it = std::find(windows.begin(), windows.end(), window);
	if (it != windows.end())
	{
		auto toPop = int(windows.end() - it) + 1; // including state_
		for (int i = 0; i < toPop; ++i)
		{
			CloseWindow();
		}
	}
}

int Engine::CloseWindow()
{
	if(!windows.empty())
	{
		frozenGraphics.pop();
		state_ = windows.back();
		windows.pop_back();

		if(state_)
			state_->DoFocus();

		ui::Point mouseState = mousePositions.top();
		mousePositions.pop();
		if(state_)
		{
			mousexp_ = mouseState.X;
			mouseyp_ = mouseState.Y;
			state_->DoMouseMove(mousex_, mousey_, mousex_ - mousexp_, mousey_ - mouseyp_);
			mousexp_ = mousex_;
			mouseyp_ = mousey_;
		}
		ignoreEvents = true;
		ApplyFpsLimit();
		return 0;
	}
	else
	{
		state_ = nullptr;
		ApplyFpsLimit();
		return 1;
	}
}

/*void Engine::SetState(State * state)
{
	if(state_) //queue if currently in a state
		statequeued_ = state;
	else
	{
		state_ = state;
		if(state_)
			state_->DoInitialized();
	}
}*/


void Engine::Tick()
{
	if(state_ != nullptr)
	{
		state_->DoTick();
	}


	lastTick = Platform::GetTime();

	ignoreEvents = false;
	/*if(statequeued_ != NULL)
	{
		if(state_ != NULL)
		{
			state_->DoExit();
			delete state_;
			state_ = NULL;
		}
		state_ = statequeued_;
		statequeued_ = NULL;

		if(state_ != NULL)
			state_->DoInitialized();
	}*/
}

void Engine::SimTick()
{
	if (state_)
	{
		state_->DoSimTick();
	}
}

void Engine::Draw()
{
	if (!frozenGraphics.empty() && !(state_ && RectSized(state_->Position, state_->Size) == g->Size().OriginRect()))
	{
		auto &frozen = frozenGraphics.top();
		std::copy_n(frozen.screen.get(), g->Size().X * g->Size().Y, g->Data());
		if (frozen.fadeTicks <= maxFadeTicks)
		{
			// from 0x00 at 0 to about 0x54 at 20
			auto alpha = uint8_t((1 - std::pow(0.98, frozen.fadeTicks)) * 0xFF);
			g->BlendFilledRect(g->Size().OriginRect(), 0x000000_rgb .WithAlpha(alpha));
		}
		// If this is the last frame in the fade, save what the faded image looks like
		if (frozen.fadeTicks == maxFadeTicks)
			std::copy_n(g->Data(), g->Size().X * g->Size().Y, frozen.screen.get());
		if (frozen.fadeTicks <= maxFadeTicks)
			frozen.fadeTicks++;
	}
	else
	{
		g->Clear();
	}
	if(state_)
		state_->DoDraw();

	g->Finalise();
	FrameIndex++;
	FrameIndex %= 7200;
}

void Engine::onKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (state_ && !ignoreEvents)
		state_->DoKeyPress(key, scan, repeat, shift, ctrl, alt);
}

void Engine::onKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt)
{
	if (state_ && !ignoreEvents)
		state_->DoKeyRelease(key, scan, repeat, shift, ctrl, alt);
}

void Engine::onTextInput(String text)
{
	if (textInput)
	{
		if (state_ && !ignoreEvents)
			state_->DoTextInput(text);
	}
}

void Engine::onTextEditing(String text, int start)
{
	if (textInput)
	{
		// * SDL sends the candidate string in packets of some arbitrary size,
		//   leaving it up to the user to assemble these packets into the
		//   complete candidate string. The start parameter tells us which
		//   portion of the candidate string the current packet spans.
		// * Sadly, there's no documented way to tell the first or last packet
		//   apart from the rest. While there's also no documented guarantee
		//   that the packets come in order and that there are no gaps or
		//   overlaps between them, the implementation on the SDL side seems to
		//   ensure this. So what we do is just append whatever packet we get
		//   to a buffer, which we reset every time a "first-y looking" packet
		//   arrives. We also forward a textediting event on every packet,
		//   which is redundant, but should be okay, as textediting events are
		//   not supposed to have an effect on the actual text being edited.
		// * We define a first-y looking packet as one with a start parameter
		//   lower than or equal to the start parameter of the previous packet.
		//   This is general enough that it seems to work around the bugs
		//   of all SDL input method backends.
		if (start <= lastTextEditingStart)
		{
			textEditingBuf.clear();
		}
		lastTextEditingStart = start;
		textEditingBuf.append(text);
		if (state_ && !ignoreEvents)
			state_->DoTextEditing(textEditingBuf);
	}
}

void Engine::onMouseDown(int x, int y, unsigned button)
{
	mouseb_ |= button;
	if (state_ && !ignoreEvents)
		state_->DoMouseDown(x, y, button);
}

void Engine::onMouseUp(int x, int y, unsigned button)
{
	mouseb_ &= ~button;
	if (state_ && !ignoreEvents)
		state_->DoMouseUp(x, y, button);
}

void Engine::initialMouse(int x, int y)
{
	mousexp_ = x;
	mouseyp_ = y;
}

void Engine::onMouseMove(int x, int y)
{
	mousex_ = x;
	mousey_ = y;
	if (state_ && !ignoreEvents)
	{
		state_->DoMouseMove(x, y, mousex_ - mousexp_, mousey_ - mouseyp_);
	}
	mousexp_ = x;
	mouseyp_ = y;
}

void Engine::onMouseWheel(int x, int y, int delta)
{
	if (state_ && !ignoreEvents)
		state_->DoMouseWheel(x, y, delta);
}

void Engine::onClose()
{
	if (state_)
		state_->DoExit();
}

void Engine::onFileDrop(ByteString filename)
{
	if (state_)
		state_->DoFileDrop(filename);
}

void Engine::StartTextInput()
{
	if (textInput)
	{
		return;
	}
	textInput = true;
	::StartTextInput();
}

void Engine::StopTextInput()
{
	if (!textInput)
	{
		return;
	}
	::StopTextInput();
	textInput = false;
}

void Engine::TextInputRect(Point position, Point size)
{
	::SetTextInputRect(position.X, position.Y, size.X, size.Y);
}

std::optional<int> Engine::GetEffectiveDrawCap() const
{
	auto drawLimit = GetDrawingFrequencyLimit();
	std::optional<int> effectiveDrawCap;
	if (auto *drawLimitExplicit = std::get_if<DrawLimitExplicit>(&drawLimit))
	{
		effectiveDrawCap = drawLimitExplicit->value;
	}
	if (std::get_if<DrawLimitDisplay>(&drawLimit))
	{
		effectiveDrawCap = std::visit([](auto &&refreshRate) {
			return refreshRate.value;
		}, GetRefreshRate());
	}
	return effectiveDrawCap;
}

void Engine::SetFps(float newFps)
{
	if (state_)
	{
		return state_->SetFps(newFps);
	}
}

float Engine::GetFps() const
{
	if (state_)
	{
		return state_->GetFps();
	}
	return 1;
}

FpsLimit Engine::GetFpsLimit() const
{
	if (state_)
	{
		return state_->GetFpsLimit();
	}
	return FpsLimitNone{};
}

bool Engine::GetContributesToFps() const
{
	if (state_)
	{
		return state_->contributesToFps;
	}
	return false;
}
