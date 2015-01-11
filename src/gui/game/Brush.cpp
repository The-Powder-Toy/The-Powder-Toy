#include "Brush.h"
#include "graphics/Renderer.h"

void Brush::RenderRect(Renderer * ren, ui::Point position1, ui::Point position2)
{
	int width, height;
	width = position2.X-position1.X;
	height = position2.Y-position1.Y;
	if(height<0)
	{
		position1.Y += height;
		height *= -1;
	}
	if(width<0)
	{
		position1.X += width;
		width *= -1;
	}
	
	ren->xor_line(position1.X, position1.Y, position1.X+width, position1.Y);
	if(height>0){
		ren->xor_line(position1.X, position1.Y+height, position1.X+width, position1.Y+height);
		if(height>1){
			ren->xor_line(position1.X+width, position1.Y+1, position1.X+width, position1.Y+height-1);
			if(width>0)
				ren->xor_line(position1.X, position1.Y+1, position1.X, position1.Y+height-1);
		}
	}
}

void Brush::RenderLine(Renderer * ren, ui::Point position1, ui::Point position2)
{
	ren->xor_line(position1.X, position1.Y, position2.X, position2.Y);
}

void Brush::RenderPoint(Renderer * ren, ui::Point position)
{
	if(!outline)
		updateOutline();
	if(!outline)
		return;
	ren->xor_bitmap(outline, position.X-radius.X, position.Y-radius.Y, size.X, size.Y);
}

void Brush::RenderFill(Renderer * ren, ui::Point position)
{
	ren->xor_line(position.X-5, position.Y, position.X-1, position.Y);
	ren->xor_line(position.X+5, position.Y, position.X+1, position.Y);
	ren->xor_line(position.X, position.Y-5, position.X, position.Y-1);
	ren->xor_line(position.X, position.Y+5, position.X, position.Y+1);
}
