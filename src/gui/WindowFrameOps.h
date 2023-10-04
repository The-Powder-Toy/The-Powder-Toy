#pragma once

struct WindowFrameOps
{
	bool resizable;
	bool fullscreen;
	bool changeResolution;
	bool forceIntegerScaling;

	bool operator ==(const WindowFrameOps &other) const
	{
		if (resizable  != other.resizable ) return false;
		if (fullscreen != other.fullscreen) return false;
		if (fullscreen)
		{
			if (changeResolution    != other.changeResolution   ) return false;
			if (forceIntegerScaling != other.forceIntegerScaling) return false;
		}
		return true;
	}

	bool operator !=(const WindowFrameOps &other) const
	{
		return !(*this == other);
	}
};
