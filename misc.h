#ifndef UTILS_H
#define UTILS_H

//Signum function
#ifdef WIN32
extern _inline int sign(float i);
#else
extern inline int sign(float i);
#endif

#ifdef WIN32
extern _inline unsigned clamp_flt(float f, float min, float max);
#else
extern inline unsigned clamp_flt(float f, float min, float max);
#endif

#ifdef WIN32
extern _inline float restrict_flt(float f, float min, float max);
#else
extern inline float restrict_flt(float f, float min, float max);
#endif

#endif