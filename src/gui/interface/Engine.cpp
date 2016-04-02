#include <iostream>
#include <stack>
#include <cstdio>
#include <cmath>

#include "Config.h"
#include "Platform.h"
#include "gui/interface/Window.h"
#include "gui/interface/Engine.h"
#include "graphics/Graphics.h"

using namespace ui;
using namespace std;

Engine::Engine():
	FpsLimit(60.0f),
	Scale(1),
	Fullscreen(false),
	Depth3d(0),
	FrameIndex(0),
	lastBuffer(NULL),
	prevBuffers(stack<pixel*>()),
	windows(stack<Window*>()),
	mousePositions(stack<Point>()),
	state_(NULL),
	windowTargetPosition(0, 0),
	break_(false),
	FastQuit(1),
	lastTick(0),
	mouseb_(0),
	mousex_(0),
	mousey_(0),
	mousexp_(0),
	mouseyp_(0),
	maxWidth(0),
	maxHeight(0)
{
}

Engine::~Engine()
{
	delete state_;
	//Dispose of any Windows.
	while(!windows.empty())
	{
		delete windows.top();
		windows.pop();
	}
	free(lastBuffer);
}

void Engine::Begin(int width, int height)
{
	//engine is now ready
	running_ = true;

	width_ = width;
	height_ = height;
}

void Engine::Break()
{
	break_ = true;
}

void Engine::UnBreak()
{
	break_ = false;
}

void Engine::Exit()
{
	running_ = false;
}

void Engine::ShowWindow(Window * window)
{
	windowOpenState = 0;
	if(window->Position.X==-1)
	{
		window->Position.X = (width_-window->Size.X)/2;
	}
	if(window->Position.Y==-1)
	{
		window->Position.Y = (height_-window->Size.Y)/2;
	}
	/*if(window->Position.Y > 0)
	{
		windowTargetPosition = window->Position;
		window->Position = Point(windowTargetPosition.X, height_);
	}*/
	if(state_)
	{
		if(lastBuffer)
		{
			prevBuffers.push(lastBuffer);
		}
		lastBuffer = (pixel*)malloc((width_ * height_) * PIXELSIZE);

#ifndef OGLI
		memcpy(lastBuffer, g->vid, (width_ * height_) * PIXELSIZE);
#endif

		windows.push(state_);
		mousePositions.push(ui::Point(mousex_, mousey_));
	}
	if(state_)
		state_->DoBlur();

	state_ = window;

}

int Engine::CloseWindow()
{
	if(!windows.empty())
	{
		if (lastBuffer)
		{
			free(lastBuffer);
			lastBuffer = NULL;
		}
		if(!prevBuffers.empty())
		{
			lastBuffer = prevBuffers.top();
			prevBuffers.pop();
		}
		state_ = windows.top();
		windows.pop();

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
		return 0;
	}
	else
	{
		state_ = NULL;
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

void Engine::SetSize(int width, int height)
{
	width_ = width;
	height_ = height;
}

void Engine::SetMaxSize(int width, int height)
{
	maxWidth = width;
	maxHeight = height;
}

void Engine::Tick()
{
	if(state_ != NULL)
		state_->DoTick(dt);


	lastTick = Platform::GetTime();

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

void Engine::Draw()
{
	if(lastBuffer && !(state_ && state_->Position.X == 0 && state_->Position.Y == 0 && state_->Size.X == width_ && state_->Size.Y == height_))
	{
		g->Acquire();
		g->Clear();
#ifndef OGLI
		memcpy(g->vid, lastBuffer, (width_ * height_) * PIXELSIZE);
		if(windowOpenState < 20)
			windowOpenState++;
		g->fillrect(0, 0, width_, height_, 0, 0, 0, 255-std::pow(.98, windowOpenState)*255);
#endif
	}
	else
	{
		g->Clear();
	}
	if(state_)
		state_->DoDraw();

	g->Finalise();
	g->Release();
	FrameIndex++;
	FrameIndex %= 7200;
}

void Engine::SetFps(float fps)
{
	this->fps = fps;
	if(FpsLimit > 2.0f)
		this->dt = 60/fps;
	else
		this->dt = 1.0f;
}

void Engine::onKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(state_)
		state_->DoKeyPress(key, character, shift, ctrl, alt);
}

void Engine::onKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt)
{
	if(state_)
		state_->DoKeyRelease(key, character, shift, ctrl, alt);
}

void Engine::onMouseClick(int x, int y, unsigned button)
{
	mouseb_ |= button;
	if(state_)
		state_->DoMouseDown(x, y, button);
}

void Engine::onMouseUnclick(int x, int y, unsigned button)
{
	mouseb_ &= ~button;
	if(state_)
		state_->DoMouseUp(x, y, button);
}

void Engine::onMouseMove(int x, int y)
{
	mousex_ = x;
	mousey_ = y;
	if(state_)
	{
		state_->DoMouseMove(x, y, mousex_ - mousexp_, mousey_ - mouseyp_);
	}
	mousexp_ = x;
	mouseyp_ = y;
}

void Engine::onMouseWheel(int x, int y, int delta)
{
	if(state_)
		state_->DoMouseWheel(x, y, delta);
}

void Engine::onResize(int newWidth, int newHeight)
{
	SetSize(newWidth, newHeight);
}

void Engine::onClose()
{
	if(state_)
		state_->DoExit();
}
