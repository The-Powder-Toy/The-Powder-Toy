#ifndef BRUSH_H_
#define BRUSH_H_
#include "Config.h"

#include "gui/interface/Point.h"

class Renderer;
class Brush
{
protected:
	unsigned char * outline;
	unsigned char * bitmap;
	ui::Point size;
	ui::Point radius;
	void updateOutline();
public:
	Brush(ui::Point size);

	//Radius of the brush 0x0 - infxinf (Radius of 0x0 would be 1x1, radius of 1x1 would be 3x3)
	ui::Point GetRadius()
	{
		return radius;
	}

	//Size of the brush bitmap mask, 1x1 - infxinf
	ui::Point GetSize()
	{
		return size;
	}
	virtual void SetRadius(ui::Point radius);
	virtual ~Brush();
	virtual void RenderRect(Renderer * ren, ui::Point position1, ui::Point position2);
	virtual void RenderLine(Renderer * ren, ui::Point position1, ui::Point position2);
	virtual void RenderPoint(Renderer * ren, ui::Point position);
	virtual void RenderFill(Renderer * ren, ui::Point position);
	virtual void GenerateBitmap();
	//Get a bitmap for drawing particles
	unsigned char * GetBitmap();

	unsigned char * GetOutline();
};


#endif /* BRUSH_H_ */
