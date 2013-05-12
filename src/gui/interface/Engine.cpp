#include <iostream>
#include <stack>
#include <cstdio>
#include <time.h>

#include "Config.h"
#include "gui/interface/Window.h"
#include "gui/interface/Platform.h"
#include "gui/interface/Engine.h"
#include "graphics/Graphics.h"

using namespace ui;
using namespace std;

Engine::Engine():
	state_(NULL),
	maxWidth(0),
	maxHeight(0),
	mouseb_(0),
	mousex_(0),
	mousey_(0),
	mousexp_(0),
	mouseyp_(0),
	FpsLimit(60.0f),
	windows(stack<Window*>()),
	mousePositions(stack<Point>()),
	lastBuffer(NULL),
	prevBuffers(stack<pixel*>()),
	windowTargetPosition(0, 0),
	FrameIndex(0),
	Fullscreen(false),
	Scale(1),
	FastQuit(1),
	break_(false),
	lastTick(0)
{
}

Engine::~Engine()
{
	if(state_ != NULL)
		delete state_;
	//Dispose of any Windows.
	while(!windows.empty())
	{
		delete windows.top();
		windows.pop();
	}
	if (lastBuffer)
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
	windowOpenState = 0.0f;
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


	lastTick = clock();
	if(windowOpenState<1.0f)
	{
		if(lastBuffer)
		{
			pixel * vid = g->vid;
			g->vid = lastBuffer;
			g->fillrect(0, 0, width_, height_, 0, 0, 0, 5);
			g->vid = vid;

		}
		/*if(windowTargetPosition.Y < state_->Position.Y)
		{
			state_->Position.Y += windowTargetPosition.Y/20;
		}*/
		windowOpenState += 0.05f;//*dt;
	}

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
	if(lastBuffer && !(state_->Position.X == 0 && state_->Position.Y == 0 && state_->Size.X == width_ && state_->Size.Y == height_))
	{
		g->Acquire();
		g->Clear();
#ifndef OGLI
		memcpy(g->vid, lastBuffer, (width_ * height_) * PIXELSIZE);
#endif
	}
	else
	{
		g->Clear();
	}
	if(state_)
		state_->DoDraw();

#ifdef DEBUG
	char fpsText[512];
	sprintf(fpsText, "FPS: %.2f, Delta: %.3f", fps, dt);
	ui::Engine::Ref().g->drawtext(10, 10, fpsText, 255, 255, 255, 255);
#endif
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
