#pragma once

namespace ui
{
	
	struct Border
	{
#if ENABLE_FLOAT_UI
# define BORDER_T float
#else
# define BORDER_T int
#endif
		
		BORDER_T Top;
		BORDER_T Right;
		BORDER_T Bottom;
		BORDER_T Left;
		
		Border(BORDER_T all):
		Top(all),
		Right(all),
		Bottom(all),
		Left(all)
		{
		}
		
		Border(BORDER_T v, BORDER_T h):
		Top(v),
		Right(h),
		Bottom(v),
		Left(h)
		{
		}
		
		Border(BORDER_T top, BORDER_T right, BORDER_T bottom, BORDER_T left):
		Top(top),
		Right(right),
		Bottom(bottom),
		Left(left)
		{
		}

		inline bool operator == (const int& v) const
		{
			return (Top == v && Right == v && Bottom == v && Left == v);
		}
		
		inline bool operator == (const Border& v) const
		{
			return (Top == v.Top && Right == v.Right && Bottom == v.Bottom && Left == v.Left);
		}
		
		inline bool operator != (const Border& v) const
		{
			return (Top != v.Top || Right != v.Right || Bottom != v.Bottom || Left != v.Left);
		}
		
		inline void operator = (const Border& v)
		{
			Top = v.Top;
			Right = v.Right;
			Bottom = v.Bottom;
			Left = v.Left;
		}
		
	};
	
}
