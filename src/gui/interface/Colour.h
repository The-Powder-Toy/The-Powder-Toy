#ifndef COLOUR_H
#define COLOUR_H

namespace ui
{
class Colour
{
public:
	unsigned char Red, Green, Blue, Alpha;
	Colour(unsigned char red, unsigned char green, unsigned char blue):
			Red(red), Green(green), Blue(blue), Alpha(255)
	{
	}
	Colour(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha):
			Red(red), Green(green), Blue(blue), Alpha(alpha)
	{
	}
	Colour()
	{
	}
};
}

#endif
