#ifndef COLOUR_H
#define COLOUR_H

namespace ui
{
class Colour
{
public:
	unsigned char Red, Green, Blue;
	Colour(unsigned char red, unsigned char green, unsigned char blue):
		Red(red), Green(green), Blue(blue)
	{
	}
	Colour()
	{
	}
};
}

#endif
