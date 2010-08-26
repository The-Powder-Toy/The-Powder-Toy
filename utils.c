#include "utils.h"

//Signum function
#ifdef WIN32
_inline int sign(float i)
#else
inline int sign(float i)
#endif
{
    if (i<0)
        return -1;
    if (i>0)
        return 1;
    return 0;
}

#ifdef WIN32
_inline unsigned clamp_flt(float f, float min, float max)
#else
inline unsigned clamp_flt(float f, float min, float max)
#endif
{
    if(f<min)
        return 0;
    if(f>max)
        return 255;
    return (int)(255.0f*(f-min)/(max-min));
}

#ifdef WIN32
_inline float restrict_flt(float f, float min, float max)
#else
inline float restrict_flt(float f, float min, float max)
#endif
{
    if(f<min)
        return min;
    if(f>max)
        return max;
    return f;
}